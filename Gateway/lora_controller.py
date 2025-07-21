from construct import Struct, Float32l, Int8ub, Int16ul
import time

from lora import lora


import mqtt_controller

from CowModel import CowModel
from group_controller import group_dict
from GroupModel import GroupModel, Point
import constants
import cow_controler

ai_gps_frame_t = Struct (
    "cow_addr" / Int16ul,
    "latitude" / Float32l,
    "longitude" / Float32l,
    "cow_status" / Int8ub,
)

ai_frame_t = Struct (
    "cow_addr" / Int16ul,
    "cow_status" / Int8ub,
)

def handle_lora_msg(msg: list) -> bool:
    print("called handle_lora_msg:", msg)
    msg_bytes = b''.join(msg)

    ai_gps_frame = None
    ai_frame = None
    if len(msg_bytes) == 11:
        ai_gps_frame = ai_gps_frame_t.parse(msg_bytes)
        print(ai_gps_frame)
    elif len(msg_bytes) == 3:
        ai_frame = ai_frame_t.parse(msg_bytes)
        print(ai_frame)
    else:
        print("Invalid lora_msg with len:", len(msg_bytes))
        return
    

    
    cow_addr = ai_gps_frame.cow_addr if ai_gps_frame != None else ai_frame.cow_addr
    print("cow addr:", cow_addr)
    # Find cow id by cow_addr
    cow: CowModel = cow_controler.get_cow_by_addr(cow_addr)

    if cow == None:
        return
    
    if ai_gps_frame != None:
        is_out_of_safe_zone = 0
        # Check gps in safezone
        if(cow.group_id != None):
            group_id: str = None
            for key in group_dict.keys():
                if(group_dict[key].group_id == cow.group_id):
                    group_id = key
                    break
            groupModel: GroupModel = group_dict[group_id]
            point: Point = Point(ai_gps_frame.longitude, ai_gps_frame.latitude)
            if(GroupModel.is_point_in_safe_zone(point) == False):
                is_out_of_safe_zone = 1
        
        print("send_cow_gps_and_status")
        mqtt_controller.send_cow_gps_and_status(
            cow_id=cow.cow_id, 
            longitude=ai_gps_frame.longitude, 
            latitude=ai_gps_frame.latitude, 
            is_out_of_safe_zone=is_out_of_safe_zone,
            cow_status=ai_gps_frame.cow_status)
        
    if ai_frame != None:
        print("send_cow_status")
        mqtt_controller.send_cow_status(
            cow_id=cow.cow_id, 
            cow_status=ai_frame.cow_status)
    
def read_lora_data():
    while(True):
        lora.read()


#register handle_lora_msg
lora.register_handle_lora_msg(handle_lora_msg)

if __name__ == "__main__":
    read_lora_data()