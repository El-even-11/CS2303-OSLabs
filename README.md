# SJTU CS2303 OSLABS

This is a repo for SJTU CS2303 Operating System Labs.

## NOTICE

Test file for RAS should **NOT** write too many times. This would result in runtime exceeding. Throttling strategy in RT solved this problem. However, I didn't implement it in RAS, and the RAS always runs in a FIFO way, which pains me a lot :(

## TODO

- Project 1
    - [x] Android Process Tree
    - [x] Pstree test
    - [x] Pstree test with student id
    - [x] Encryption server and client
- Project 2
    - [x] Write tracer
    - [x] Basic ras scheduler and test file (May. 6th) 
    - [x] Weighted timeslice determined by race prob (May. 6th) 
    - [x] Benchmark (May. 7th) 
    - [ ] Optimize race probability calculation O(n) -> O(1)
    - [ ] Throttling strategy
    - [ ] Run on Multi-CPU
    