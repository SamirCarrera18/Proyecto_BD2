#!/usr/bin/env python3
"""CSV INTEGER reproducible: D1 secuencial o D2 aleatorio único."""
import argparse,random
p=argparse.ArgumentParser()
p.add_argument('--n',type=int,default=100000)
p.add_argument('--seed',type=int,default=20260911)
p.add_argument('--mode',choices=['sequential','random'],default='random')
a=p.parse_args()
if not 1 <= a.n <= 1000000:p.error('n debe estar entre 1 y 1000000')
keys=range(1,a.n+1) if a.mode=='sequential' else random.Random(a.seed).sample(range(1,10000001),a.n)
for k in keys:print(k)
