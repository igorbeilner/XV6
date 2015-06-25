4300 // Buffer cache.
4301 //
4302 // The buffer cache is a linked list of buf structures holding
4303 // cached copies of disk block contents.  Caching disk blocks
4304 // in memory reduces the number of disk reads and also provides
4305 // a synchronization point for disk blocks used by multiple processes.
4306 //
4307 // Interface:
4308 // * To get a buffer for a particular disk block, call bread.
4309 // * After changing buffer data, call bwrite to write it to disk.
4310 // * When done with the buffer, call brelse.
4311 // * Do not use the buffer after calling brelse.
4312 // * Only one process at a time can use a buffer,
4313 //     so do not keep them longer than necessary.
4314 //
4315 // The implementation uses three state flags internally:
4316 // * B_BUSY: the block has been returned from bread
4317 //     and has not been passed back to brelse.
4318 // * B_VALID: the buffer data has been read from the disk.
4319 // * B_DIRTY: the buffer data has been modified
4320 //     and needs to be written to disk.
4321 
4322 #include "types.h"
4323 #include "defs.h"
4324 #include "param.h"
4325 #include "spinlock.h"
4326 #include "buf.h"
4327 
4328 struct {
4329   struct spinlock lock;
4330   struct buf buf[NBUF];
4331 
4332   // Linked list of all buffers, through prev/next.
4333   // head.next is most recently used.
4334   struct buf head;
4335 } bcache;
4336 
4337 void
4338 binit(void)
4339 {
4340   struct buf *b;
4341 
4342   initlock(&bcache.lock, "bcache");
4343 
4344 
4345 
4346 
4347 
4348 
4349 
4350   // Create linked list of buffers
4351   bcache.head.prev = &bcache.head;
4352   bcache.head.next = &bcache.head;
4353   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
4354     b->next = bcache.head.next;
4355     b->prev = &bcache.head;
4356     b->dev = -1;
4357     bcache.head.next->prev = b;
4358     bcache.head.next = b;
4359   }
4360 }
4361 
4362 // Look through buffer cache for sector on device dev.
4363 // If not found, allocate a buffer.
4364 // In either case, return B_BUSY buffer.
4365 static struct buf*
4366 bget(uint dev, uint sector)
4367 {
4368   struct buf *b;
4369 
4370   acquire(&bcache.lock);
4371 
4372  loop:
4373   // Is the sector already cached?
4374   for(b = bcache.head.next; b != &bcache.head; b = b->next){
4375     if(b->dev == dev && b->sector == sector){
4376       if(!(b->flags & B_BUSY)){
4377         b->flags |= B_BUSY;
4378         release(&bcache.lock);
4379         return b;
4380       }
4381       sleep(b, &bcache.lock);
4382       goto loop;
4383     }
4384   }
4385 
4386   // Not cached; recycle some non-busy and clean buffer.
4387   // "clean" because B_DIRTY and !B_BUSY means log.c
4388   // hasn't yet committed the changes to the buffer.
4389   for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
4390     if((b->flags & B_BUSY) == 0 && (b->flags & B_DIRTY) == 0){
4391       b->dev = dev;
4392       b->sector = sector;
4393       b->flags = B_BUSY;
4394       release(&bcache.lock);
4395       return b;
4396     }
4397   }
4398   panic("bget: no buffers");
4399 }
4400 // Return a B_BUSY buf with the contents of the indicated disk sector.
4401 struct buf*
4402 bread(uint dev, uint sector)
4403 {
4404   struct buf *b;
4405 
4406   b = bget(dev, sector);
4407   if(!(b->flags & B_VALID))
4408     iderw(b);
4409   return b;
4410 }
4411 
4412 // Write b's contents to disk.  Must be B_BUSY.
4413 void
4414 bwrite(struct buf *b)
4415 {
4416   if((b->flags & B_BUSY) == 0)
4417     panic("bwrite");
4418   b->flags |= B_DIRTY;
4419   iderw(b);
4420 }
4421 
4422 // Release a B_BUSY buffer.
4423 // Move to the head of the MRU list.
4424 void
4425 brelse(struct buf *b)
4426 {
4427   if((b->flags & B_BUSY) == 0)
4428     panic("brelse");
4429 
4430   acquire(&bcache.lock);
4431 
4432   b->next->prev = b->prev;
4433   b->prev->next = b->next;
4434   b->next = bcache.head.next;
4435   b->prev = &bcache.head;
4436   bcache.head.next->prev = b;
4437   bcache.head.next = b;
4438 
4439   b->flags &= ~B_BUSY;
4440   wakeup(b);
4441 
4442   release(&bcache.lock);
4443 }
4444 
4445 
4446 
4447 
4448 
4449 
4450 // Blank page.
4451 
4452 
4453 
4454 
4455 
4456 
4457 
4458 
4459 
4460 
4461 
4462 
4463 
4464 
4465 
4466 
4467 
4468 
4469 
4470 
4471 
4472 
4473 
4474 
4475 
4476 
4477 
4478 
4479 
4480 
4481 
4482 
4483 
4484 
4485 
4486 
4487 
4488 
4489 
4490 
4491 
4492 
4493 
4494 
4495 
4496 
4497 
4498 
4499 
