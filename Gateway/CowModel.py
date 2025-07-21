class CowModel:
    def __init__(
            self, 
            cow_id: str, 
            cow_addr: int | None,
            group_id: str | None,
        ):
        
        self.cow_id = cow_id
        self.cow_addr = cow_addr
        self.group_id = group_id
