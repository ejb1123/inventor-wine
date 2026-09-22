package main

import (
	"fmt"
	"os"
	"unsafe"

	"golang.org/x/sys/windows"
	"golang.org/x/sys/windows/svc"
)

type handler struct{}

func (handler) Execute(args []string, req <-chan svc.ChangeRequest, out chan<- svc.Status) (bool, uint32) {
	out <- svc.Status{State: svc.Running, Accepts: svc.AcceptStop}
	for r := range req {
		if r.Cmd == svc.Stop {
			break
		}
		if r.Cmd == svc.Interrogate {
			out <- r.CurrentStatus
		}
	}
	out <- svc.Status{State: svc.StopPending}
	return false, 0
}
func main() {
	isService, err := svc.IsWindowsService()
	interactive, ierr := svc.IsAnInteractiveSession()
	f, fileErr := os.OpenFile(`C:\service-probe.txt`, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0600)
	if fileErr != nil {
		fmt.Fprintln(os.Stderr, fileErr)
		os.Exit(2)
	}
	defer f.Close()
	fmt.Fprintf(f, "pid=%d IsWindowsService=%v err=%v interactive=%v err=%v\n", os.Getpid(), isService, err, interactive, ierr)
	var p windows.PROCESS_BASIC_INFORMATION
	var n uint32
	if err := windows.NtQueryInformationProcess(windows.CurrentProcess(), windows.ProcessBasicInformation, unsafe.Pointer(&p), uint32(unsafe.Sizeof(p)), &n); err != nil {
		fmt.Fprintf(f, "query parent: %v\n", err)
		os.Exit(2)
	}
	var sid uint32
	err = windows.ProcessIdToSessionId(uint32(p.InheritedFromUniqueProcessId), &sid)
	fmt.Fprintf(f, "parent=%d session=%d err=%v\n", p.InheritedFromUniqueProcessId, sid, err)
	err = svc.Run("InventorProbe", handler{})
	fmt.Fprintf(f, "svc.Run returned %v\n", err)
}
