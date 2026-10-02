import xml.etree.ElementTree as ET

tree = ET.parse("Charity.xml")
root = tree.getroot()
for cell in root.findall(".//mxCell"):
    if cell.attrib.get("edge") == "1":
        source = cell.attrib.get("source", "")
        target = cell.attrib.get("target", "")
        parent = cell.attrib.get("parent", "")
        value = cell.attrib.get("value", "")
        
        # In DrawIO, edges might be wrapped in UserObject for labels
        # Let's check parent
        for uo in root.findall(".//UserObject"):
            if uo.attrib.get("id") == parent:
                if not value:
                    value = uo.attrib.get("label", "")
                break
        # Sometimes the UserObject IS the edge wrapper, but cell is its child.
        print(f"Edge ID={cell.attrib.get('id')}: source={source}, target={target}, value={value}, parent={parent}")
