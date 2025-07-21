from CowModel import CowModel

cows_dict = {}

def add_cow(cowModel: CowModel):
    cows_dict[cowModel.cow_id] = cowModel

def delete_cow(cow_id: str):
    del cows_dict[cow_id]

def update_cow(cowModel: CowModel):
    cows_dict[cowModel.cow_id] = cowModel

def get_cow_by_addr(cow_addr):
    for key in cows_dict.keys():
        if cows_dict[key].cow_addr == cow_addr:
            return cows_dict[key]
    return None

def print_cow_dict():
    for key in cows_dict.keys():
        cowModel: CowModel = cows_dict[key]
        print("cow_id:", cowModel.cow_id)
        print("    cow_addr:", cowModel.cow_addr)
        print("    group_id:", cowModel.group_id)