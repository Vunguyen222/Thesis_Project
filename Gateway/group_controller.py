from GroupModel import GroupModel
group_dict = {}

def add_group(groupModel: GroupModel):
    group_dict[groupModel.group_id] = groupModel

def delete_safe_zone(group_id: str):
    del group_dict[group_id]

def update_safe_zone(groupModel: GroupModel):
    group_dict[groupModel.group_id] = groupModel

def print_group_dict():
    for key in group_dict.keys():
        groupModel: GroupModel = group_dict[key]

        print("group_id:", groupModel.group_id)
        print("points")
        for point in groupModel.safe_zone_points:
            print(f"    ({point.latitude}, {point.longitude})")