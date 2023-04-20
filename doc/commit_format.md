stub is ok.

Description:
i have add a stub admin to alloc stub. when free stub
we add stub to free list. when alloc, get from free stub
list first.

Major Changes:
1. modifi list::detach_front.
2. modify allocator's log level of print list
3. add stub admin
4. stub has passed test cases.