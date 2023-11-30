[update:object] implement calling the methods of the subclass at parent class.

Description:
Inorder to support calling zip open func at archive object, i have add a new
object api(object_get_progeny_first_normal_func).

Major Changes:
1. implement object_get_progeny_first_normal_func.
2. rename object_get_progeny_last_reimplement_func from __object_find_reimplement_func and change par order.
3. rename object_get_func_of_class from __object_get_func_of_class.
4. support calling zip open func at archive object.