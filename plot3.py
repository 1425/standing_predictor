#!/usr/bin/env python

#How to do this:
#Z-axis=probability
#X-axis=skill measure in (either pts or OPR rank)
#Y-axis=skill measure out (aka pts)
#
#input format: lines of "x,y,z" for each point?

import matplotlib.pyplot as plt
import numpy as np

from matplotlib import cm
from matplotlib.ticker import LinearLocator
from argparse import ArgumentParser
import sys

def plot(data,xlabel,ylabel,zlabel):

    fig, ax = plt.subplots(subplot_kw={"projection": "3d"},figsize=(8,8))

    #X = list(map(lambda x: x[0],data))
    #Y = list(map(lambda x: x[1],data))
    xs=list(set(map(lambda x: x[0],data)))
    ys=list(set(map(lambda x: x[1],data)))

    X=np.array(range(min(xs),max(xs)+1))
    Y=np.array(range(min(ys),max(ys)+1))
    X, Y = np.meshgrid(X, Y)

    def get_z(x,y):
        f=list(filter(lambda item: item[0]==x and item[1]==y,data))
        if len(f)==0:
            return 0
        if len(f)==1:
            return f[0][2]
        raise 'error'

    z2={}
    for x,y,z in data:
        z2[(x,y)]=z

    #print(z2)

    def get_z2(x,y):
        try:
            return z2[(x,y)]
        except:
            return 0

    vec_f=np.vectorize(get_z2)

    Z=vec_f(X,Y)

    #surf = ax.plot_surface(X, Y, Z, cmap=cm.coolwarm,
    #                   linewidth=0, antialiased=False)
    surf = ax.plot_surface(X, Y, Z, cmap=cm.coolwarm,
                       linewidth=1, antialiased=True)

    ax.set_xlim(0,max(xs))
    ax.set_ylim(0,max(ys))
    ax.invert_xaxis()
    #ax.invert_yaxis()
    #plt.ylim(bottom=0)
    #plt.xlim(bottom=0)


#ax.set_zlim(-1.01, 1.01)
    #ax.zaxis.set_major_locator(LinearLocator(10))
    # A StrMethodFormatter is used automatically
    ax.zaxis.set_major_formatter('{x:.02f}')

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_zlabel(zlabel)

    # Add a color bar which maps values to colors.
    #fig.colorbar(surf, shrink=0.5, aspect=5)

    #plt.show()
    plt.savefig(sys.stdout,format='png')

parse_item=float

def parse_line(s):
    #return list(map(parse_item,s.split(',')))
    sp=s.split(',')
    return int(sp[0]),int(sp[1]),float(sp[2])

def parse_data(data):
    return list(map(parse_line,data.splitlines()))

def read():
    return parse_data(sys.stdin.read())

def main():
    a=ArgumentParser()
    a.add_argument('--x')
    a.add_argument('--y')
    a.add_argument('--z')
    args=a.parse_args()
    plot(read(),args.x,args.y,args.z)

if __name__=='__main__':
    main()
