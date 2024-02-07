[update:bus] optimize bus.

Description:
The object id can ensure the uniqueness of an object, and 
the name can be used to query objects that support the same 
feature. so we add object id for bus.

Major Changes:
1. add bus add, but the bus name is obmited.
2. support config log bussiness for mockery