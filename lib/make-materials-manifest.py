"""Create a local owner for unmodified Autodesk material-library packages.

ODIS removes standalone shared packages without an application owner. This
separate bundle lets their September certificate-valid install finish before
the main Inventor installation runs at the later date required by RSA/REX.
"""
import copy
import sys
import uuid
from pathlib import Path
from xml.etree import ElementTree as ET

NS = "https://emsfs.autodesk.com/schema/manifest/1/0"
ET.register_namespace("", NS)


def tag(name):
    return f"{{{NS}}}{name}"


def node(parent, element_name, text=None, **attrs):
    element = ET.SubElement(parent, tag(element_name), attrs)
    element.text = text
    return element


def identifier(name):
    value = uuid.uuid5(uuid.NAMESPACE_URL, "inventor-wine/material-preparation/" + name)
    return "{" + str(value).upper() + "}"


def identity(parent, name, key, upgrade, constant):
    element = node(parent, "Identity")
    for field, value in (
        ("Publisher", "Local Wine compatibility setup"),
        ("DisplayName", name), ("PLC", "WINE_MATERIAL_PREP"),
        ("Release", "1"), ("BuildNumber", "1.0.0"),
        ("UPI2", identifier(key)), ("UpgradeCode", identifier(upgrade)),
        ("ConstantId", identifier(constant)),
    ):
        node(element, field, value)
    return element


def main(image, destination):
    image = Path(image).resolve(strict=True)
    destination = Path(destination)
    destination.mkdir(parents=True, exist_ok=True)
    content = destination / "Content"
    if content.exists() or content.is_symlink():
        if not content.is_symlink() or content.resolve() != image / "Content":
            raise ValueError(f"Unexpected existing content link: {content}")
    else:
        content.symlink_to(image / "Content", target_is_directory=True)

    bundle = ET.Element(tag("Bundle"), version="1.0")
    element = identity(bundle, "Inventor Wine Material Preparation", "bundle", "upgrade", "constant")
    node(element, "Type", "PRD")
    node(bundle, "Resources")
    config = node(bundle, "Configuration")
    node(config, "Attributes")
    node(node(config, "Platforms"), "Platform", name="Windows", architecture="x64", minVersion="7.1")
    node(node(config, "Languages"), "Language", langId="en-US")

    app = ET.Element(tag("Application"), version="1.0")
    identity(app, "Inventor Wine Material Libraries", "application", "app-upgrade", "app-constant")
    node(app, "Resources")
    app.append(copy.deepcopy(config))
    packages = node(app, "Packages")
    for folder, name in (("CM", "MaterialLibrary5"), ("ILB", "BaseImageLibrary5"), ("ILL", "LowImageLibrary5")):
        relative = f"Content/ADSKMaterials/{folder}/pkg.{name}.xml"
        original = ET.parse(image / relative).getroot().find(tag("Identity"))
        def value(field):
            result = original.findtext(tag(field))
            if not result:
                raise ValueError(f"Missing {field} in {relative}")
            return result
        node(packages, "Package", installAs="core", external="true",
             name=value("DisplayName"), path=relative, upi2=value("UPI2"), upgradeCode=value("UpgradeCode"))

    node(node(bundle, "Applications"), "Application", sequence="1", installAs="core",
         external="false", name="Inventor Wine Material Libraries", path="application.xml",
         upi2=identifier("application"), upgradeCode=identifier("app-upgrade"))
    for root, filename in ((bundle, "setup.xml"), (app, "application.xml")):
        ET.indent(root)
        ET.ElementTree(root).write(destination / filename, encoding="utf-8", xml_declaration=True)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: make-materials-manifest.py IMAGE_DIRECTORY DESTINATION")
    main(*sys.argv[1:])
