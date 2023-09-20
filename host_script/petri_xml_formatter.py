from xml.etree import ElementTree

control_task_t = ['t7', 't14', 't28', 't27', 't17', 't5', 't3', 't2', 't1', 't38', 't39', 't41', 't52', 't59', 't62', 't63', 't66', 't67', 't68', 't71', 't72', 't74', 't75']
wifi_task_t = ['t11', 't32', 't19', 't31', 't26', 't18', 't30', 't23', 't10', 't9', 't8', 't6', 't37', 't40', 't44', 't47']
diagn_task_t = ['t36', 't35', 't34', 't33', 't25', 't24', 't22', 't55', 't20', 't21']


# Per ricavare le t di ogni task, va usato l'xml della rete precedente alle modifiche

# tree = ElementTree.parse("./petri.xml")
# root = tree.getroot()

# for child in root:
#     if "transition" in child.tag:
#         if "transition_resource" in child[0].tag:
#             if child[0].attrib["ref"] == "r1":
#                 control_task_t.append(child.attrib["id"])
#             elif child[0].attrib["ref"] == "r6":
#                 wifi_task_t.append(child.attrib["id"])
#             elif child[0].attrib["ref"] == "r0":
#                 diagn_task_t.append(child.attrib["id"])

tree = ElementTree.parse("./petri2.xml")
root = tree.getroot()

for transition in root.iter("transition"):
    if transition.attrib["id"] in control_task_t:
        transition[0].set("ref", "r1")
        transition.set("priority", "3")

    elif transition.attrib["id"] in wifi_task_t:
        transition[0].set("ref", "r1")
        transition.set("priority", "2")

    elif transition.attrib["id"] in diagn_task_t:
        transition[0].set("ref", "r1")
        transition.set("priority", "1")
    else :
        transition[0].set("EFT", "0")
        transition[0].set("LFT", "20")



tree.write("./petri_new.xml")

