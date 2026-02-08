#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import sys
from argparse import ArgumentParser

def parse_item(s):
    return float(s)

def parse_line(s):
    return list(map(parse_item,s.split(',')))

def parse_data(data):
    return list(map(parse_line,data.splitlines()))

def plot(info,xlabel,ylabel):
    x2=list(map(lambda x: x[0],info))
    y2=list(map(lambda x: x[1],info))
    fig, ax = plt.subplots(figsize=(8,4))
#sys.stderr=tmp
#print ('bbb')

#ax.plot(x2, y2 + 2.5, 'x', markeredgewidth=2)
#print('ccc')
#ax.plot(x, y, linewidth=2.0)
#print('ddd')
    #ax.plot(x2, y2 , 'o-', linewidth=2)
    #ax.bar(x2, y2 , linewidth=2)
    ax.bar(x2, y2)

#print('foo')
#ax.set(xlim=(0, 8), xticks=np.arange(1, 8),
#       ylim=(0, 8), yticks=np.arange(1, 8))
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    #probably want this if you have a line but not if you have a bar graph.
    #ax.set_xlim(min(x2),max(x2))

    #ax.set_ylim(min(y2),max(y2))

#plt.show()
    plt.tight_layout()
    #plt.savefig("out.svg")
    plt.savefig(sys.stdout,format='png')
#print('bar')

def main():
    a=ArgumentParser()
    a.add_argument('--x')
    a.add_argument('--y')
    args=a.parse_args()

    data=sys.stdin.read()

    plot(parse_data(data),args.x,args.y)

if __name__=='__main__':
    #print('hello')
    #exit(0)
    main()
