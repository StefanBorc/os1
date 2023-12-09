// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  /* z labu
  struct spinlock lock;
  struct buf buf[NBUF];
  */
  //doplnene
  struct spinlock lock[BUCKETS];
  struct buf buf[BUCKETS][BNBUF];
  //
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.

  /* z labu
  struct buf head;
  */
  //doplnene
  struct buf head[BUCKETS];
  //
} bcache;

void
binit(void)
{
  struct buf *b;
  /* z labu
  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }*/

  //doplnene
  for(int i=0;i<BUCKETS;++i){
    initlock(&bcache.lock[i],"bcache");
    
    //vytvor ll z bufferov
    bcache.head[i].prev=&bcache.head[i];
    bcache.head[i].next=&bcache.head[i];

    for(b=bcache.buf[i];b<bcache.buf[i]+BNBUF;b++){
      b->next=bcache.head[i].next;
      b->prev=&bcache.head[i];
      initsleeplock(&b->lock,"buffer");
      bcache.head[i].next->prev=b;
      bcache.head[i].next=b;
    }
  }
  //
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  /*z labu
   acquire(&bcache.lock);
 */
  //doplnene
  int i=blockno%BUCKETS;
  acquire(&bcache.lock[i]);
  //
  // Is the block already cached?
  //
  // z labu zmeneny head na head[i]
  for(b = bcache.head[i].next; b != &bcache.head[i]; b = b->next){
  
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      //zmeneny release lock na lock[i]
      release(&bcache.lock[i]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  
  //zmena head na head[i] 
  for(b = bcache.head[i].prev; b != &bcache.head[i]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      //release zmena lock na lock[i]
      release(&bcache.lock[i]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  //doplnene
  //b=bcache.head[i].prev;

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
  //doplnene
  int i=b->blockno%BUCKETS;
  //
  releasesleep(&b->lock);
  //zmena lock na lock[i]
  acquire(&bcache.lock[i]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it
    b->next->prev = b->prev;
    b->prev->next = b->next;

    //zmena head na head[i]
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
  }
  //zmena lock na lock[i]
  release(&bcache.lock[i]);
}

void
bpin(struct buf *b) {
  //doplnene int a zmena lock na lock[i]
  int i=b->blockno%BUCKETS;
  acquire(&bcache.lock[i]);
  b->refcnt++;
  release(&bcache.lock[i]);
}

void
bunpin(struct buf *b) {
  //zmena lock na lock[i] a doplnene int i a zmena lock na lock[i]
  int i=b->blockno%BUCKETS;
  acquire(&bcache.lock[i]);
  b->refcnt--;
  release(&bcache.lock[i]);
}


