import time
import mqtt_client_helper
import constants
import json

from CowModel import CowModel
import cow_controler

from GroupModel import GroupModel, Point
import group_controller

from cow_controler import cows_dict

def get_header(msg: str) -> int:
    if len(msg) >= 2: return int(msg[0:2])
    else: return None
    
def handle_mqtt_msg(msg: str):
    """
    This function will handle msg from mqtt
    Params:
        msg (str):
            mqtt message
    """
    # Split message to header and data

    header = get_header(msg)
    
    if(header == constants.HEADER_BACKEND_RESPONSE_GET_COWS):
        cows_data: str = msg[2:]
        data_list = json.loads(cows_data)
        
        for cow_dict in data_list:
            cow_addr: str | None = None
            group_id: str | None = None

            if("cow_addr" in cow_dict):
                cow_addr = cow_dict['cow_addr']
            if("groupId" in cow_dict):
                group_id = cow_dict['groupId']
            
            cowModel: CowModel = \
                CowModel(cow_id=cow_dict['_id'], 
                         cow_addr=cow_addr,
                         group_id=group_id
                         )
            cow_controler.add_cow(cowModel=cowModel)
        
        # print(f"After get all cows: {cow_controler.cows_dict}")
        
    elif(header == constants.HEADER_BACKEND_CREATE_COW):
        # <header><cow>
        # split to get cow_data
        cow_data: str = msg[2:]

        # convert cow_data from string to dict
        cow_dict = json.loads(cow_data)

        # add cow to global variable
        cowModel: CowModel.CowModel = CowModel.CowModel(cow_id=cow_dict['_id'], cow_addr=cow_dict['cow_addr'])
        cow_controler.add_cow(cowModel)

        # response ack to backend
        publish_mqtt_msg(f"0{constants.HEADER_GATEWAY_ACK}{cowModel.cow_id}")

        print(f"After create cow: {cow_controler.cows_dict}")
    
    elif(header == constants.HEADER_BACKEND_DELETE_COW):
        # <header><cow_id>
        # split to get cow id
        cow_id: str = msg[2:]

        #delete cow from global variable
        cow_controler.delete_cow(cow_id=cow_id)
        
        # response ack to backend
        publish_mqtt_msg(f"0{constants.HEADER_GATEWAY_ACK}")
        print(f"After delete cow: {cow_controler.cows_dict}")
    
    elif(header == constants.HEADER_BACKEND_RESPONSE_GROUPS):
        # <header><safezones>
        # split to get safezones
        groups_data: str = msg[2:]

        # load json
        data_list = json.loads(groups_data)
        # print(data_list)
        for group_dict in data_list:
            group_id: str = group_dict['groupId']
            safe_zone_points: list = []

            for point in group_dict['safeZone']:
                point: Point = Point(point['longitude'], point['latitude'])
                safe_zone_points.append(point)
            
            groupModel: GroupModel = \
                GroupModel(group_id, safe_zone_points)
            group_controller.add_group(groupModel)
        
    else:
        print(f"Header is invalid, header = {header}")
    

def publish_mqtt_msg(msg: str):
    """
    This function will publish `msg` to mqtt broker's `topic` 
    Params:
        msg (str):
            message which we would like to publish
    """
    mqtt_client_helper.mqttClientHelper.publish(msg)


def get_all_cows():
    """
    This function will send a request to get all cows information
to backend
    """

    msg: str = f"0{constants.HEADER_GATEWAY_REQUEST_GET_COWS}" \
                if constants.HEADER_GATEWAY_REQUEST_GET_COWS < 10 \
                else f"{constants.HEADER_GATEWAY_REQUEST_GET_COWS}"
    publish_mqtt_msg(msg)

def send_ack(ack: int):
    """
    This function will feedback ack to backend
    Params:
        ack (int): ack which we wanna feedback to backend
    """

    msg: str = f"0{constants.HEADER_GATEWAY_ACK}{ack}"
    publish_mqtt_msg(msg)

def send_cow_gps_and_status(cow_id:str, longitude: float, latitude: float, is_out_of_safe_zone: int, cow_status: int):
    msg: str = f"0{constants.HEADER_GATEWAY_SEND_COW_GPS_AND_STATUS}" \
                if constants.HEADER_GATEWAY_SEND_COW_GPS_AND_STATUS < 10 \
                else f"{constants.HEADER_GATEWAY_SEND_COW_GPS_AND_STATUS}"
    msg += f"{cow_id}:{longitude}:{latitude}:{is_out_of_safe_zone}:{cow_status}"
    publish_mqtt_msg(msg)

def send_cow_status(cow_id: str, cow_status: int):
    msg: str = f"0{constants.HEADER_GATEWAY_SEND_COW_STATUS}" \
                if constants.HEADER_GATEWAY_SEND_COW_STATUS < 10 \
                else f"{constants.HEADER_GATEWAY_SEND_COW_STATUS}"
    msg += f"{cow_id}:{cow_status}"
    publish_mqtt_msg(msg)
    
def send_error(cow_id: str, error_code: int):
    msg: str = f"0{constants.HEADER_GATEWAY_SEND_ERROR}" \
                if constants.HEADER_GATEWAY_SEND_ERROR < 10 \
                else f"{constants.HEADER_GATEWAY_SEND_ERROR}"
    msg += f"{cow_id}:{error_code}"
    publish_mqtt_msg(msg)


def get_safe_zones():
    """
    This function will send a request to get safe zone
    """

    msg: str =  f"0{constants.HEADER_GATEWAY_REQUEST_GROUPS}" \
                if constants.HEADER_GATEWAY_REQUEST_GROUPS < 10 \
                else f"{constants.HEADER_GATEWAY_REQUEST_GROUPS}"
    publish_mqtt_msg(msg)

if __name__ == "__main__":
    time.sleep(3)
    get_all_cows()
    time.sleep(3)
    cow_controler.print_cow_dict()
    get_safe_zones()
    time.sleep(3)
    group_controller.print_group_dict()
    