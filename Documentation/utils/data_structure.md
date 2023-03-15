# Data Structure Document

[TOC]

## Link List
## Rbtree Map
## Hash Map
## Vector
## Array Stack
## Heap
### Iterfaces
* heap_alloc
```
heap_t *heap_alloc(allocator_t *allocator)
```
* heap_set
```
int heap_set(heap_t *heap, char *key, void *value)
```
* heap_init
```
heap_t *heap_init(heap_t *heap)
```
* heap_add
```
void heap_add(heap_t *heap, void *e)
```
* heap_remove
```
int heap_remove(heap_t *heap, void **element)
```
* heap_destroy
```
int heap_destroy(heap_t *heap)
```
* heap_size
```
int heap_size(heap_t *heap)
```

### Demos
```
int test_heap() 
{ 
    heap_t *heap;
    void *element;
    int size                 = 0;
    int capacity             = 1024;
    allocator_t *allocator   = allocator_get_default_instance();
 
    heap                     = heap_alloc(allocator);
    heap_set(heap, "comparator", (void *)greater_than);
    heap_set(heap, "capacity", (void *) &capacity);
 
    heap_init(heap);
 
    heap_add(heap, (void *)4);
    heap_add(heap, (void *)3);
    heap_add(heap, (void *)7);
    heap_add(heap, (void *)2);
    heap_add(heap, (void *)3);
 
    dbg_buf(DBG_DETAIL,"heap:", (void *)heap->queue, 30);
    dbg_str(DBG_DETAIL,"size = %d", heap_size(heap));
 
    size                     = heap_size(heap);
    for(int i                = 0; i< size; i++){
    heap_remove(heap, &element);
    dbg_str(DBG_DETAIL, "%d", (long long)element);
    }
 
    heap_destroy(heap);
    return 0;
}
```

### 原理
堆（英语：heap)是计算机科学中一类特殊的数据结构的统称。堆通常是一个可以被看做一棵树的数组对象。 
堆总是满足下列性质：
堆中某个节点的值总是大于或小于其父节点的值；
堆总是一颗完全树。
将根节点最大的堆叫做最大堆或大根堆，根节点最小的堆叫做最小堆或小根堆。

#### insert
```
void shiftup(heap_t *heap, int index, void *e)
{
    while(index > 0) {
        int parent = (index - 1) / 2;
        if (heap->comparator(e, heap->queue[parent])) {
            break;
        }

        /*if parent smaller than inserting child, exchange position*/
        heap->queue[index] = heap->queue[parent];
        index = parent;
    }
    heap->queue[index] = e;
}

void heap_add(heap_t *heap, void *e)
{

    int index = heap->size;

    shiftup(heap, index, e);
    heap->size++;
}
```

#### delete
```
void shiftdown(heap_t *heap, int index, void *e)
{
    int size = heap->size;
    int half = size / 2;

    while (index < half) {
        int child = 2 * index + 1;
        int right = child + 1;
        if (    right < size &&
                heap->comparator(heap->queue[child], heap->queue[right])) {
            child = right;
        }
        if (heap->comparator(heap->queue[child], e)) {
            break;
        }

        /*save greater child to parent*/
        heap->queue[index] = heap->queue[child]; 
        index = child;
    }

    heap->queue[index] = e;
}

int heap_remove(heap_t *heap, void **element)
{
    if (heap->size <= 0)
        return -1;

    *element = heap->queue[0];

    shiftdown(heap, 0, heap->queue[heap->size - 1]);
    heap->queue[heap->size--] = 0;

    return 0;
}
```
#### sort
