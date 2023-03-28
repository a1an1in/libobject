expand Buffer class and optimze RingBuffer api paramter.

Description:
optimize RingBuffer api, and passed test after modification.
we i develop command::help, i found it need Buffer::rfind
interface. so we need to expand this class.

Major Changes:
1. implement part of command::help.
2. add Buffer::find and Buffer::rfind.
3. del useless len parameter of RingBuffer::find and RingBuffer::get_needle_offset