#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import matplotlib.dates as mdates
import datetime
from json import load
from sys import stdin,stdout

def plot(x,y,labels):
    fig, ax = plt.subplots()
    #ax.scatter(x, y)
    ax.scatter(x, y)
    ax.plot(x,y)

    # Annotate each point
    for i, label in enumerate(labels):
        ax.annotate(label, # The text label
                (x[i], y[i]), # The point's coordinates (xy)
                textcoords="offset points", # How to position the text
                xytext=(0, 10), # Distance from the point to the text (offset)
                ha='center') # Horizontal alignment
    plt.title("Progression of probabilities")
    ax.set_xlabel("P(DCMP)")
    ax.set_ylabel("P(CMP)")
    plt.tight_layout()
    plt.show()

def parse_line(s):
    return s.split(',')

def parse():
    r=[]
    for line in open('history.csv'):
        r.append(parse_line(line))
    return r

def print_lines(a):
    for x in a:
        print(x)

def plot_lines(y1,y2,x):
    #dt.datetime(2023, 1, i)
    x=list(map(lambda x: datetime.datetime.strptime(x, "%Y-%m-%d"),x))
    print(x)
    fig,ax=plt.subplots()
    ax.plot(x,y1,marker='o')
    #ax.plot(x,y2,marker='o',linestyle='dotted')
    ax.plot(x,y2,marker='o',linestyle='dotted')

    #vertical lines for dates of competitions
    #plt.axvline(datetime.datetime(2025,3,8),color='red')
    #plt.axvline(datetime.datetime(2025,3,23),color='red')
    #plt.axvline(datetime.datetime(2025,4,5),color='red')

    #ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
    plt.tight_layout()
    plt.show()

def main():
    # Sample data
    x = [1, 2, 3, 4, 5]
    y = [2, 3, 5, 4, 6]
    labels = ['A', 'B', 'C', 'D', 'E']

    p=parse()[1:]
    p=list(filter(lambda x: x[0]=='frc4990',p))
    #p=list(filter(lambda x: x[0]=='frc957',p))
    p=list(filter(lambda x: x[1]=='2026',p))
    #p=p[:20]
    print_lines(p)
    dates=[i[2] for i in p]
    dcmp_pr=[float(i[4]) for i in p]
    cmp_pr=[float(i[5]) for i in p]
    #plot(dcmp_pr,cmp_pr,dates)
    plot_lines(dcmp_pr,cmp_pr,dates)

def plot2(data):
    fig,ax=plt.subplots()
    for line in data['lines']:
        x=[datetime.datetime.strptime(i[0],'%Y-%m-%d') for i in line]
        y=[i[1] for i in line]
        fill=[[i[2] for i in line]]
        #initiallly, going to ignore the fill and plot them all solid
        ax.plot(x,y,marker='o')
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%d %b %Y'))
    plt.title(data['title'])
    ax.set_xlabel(data['x_label'])
    ax.set_ylabel(data['y_label'])
    plt.tight_layout()
    #plt.show()
    plt.savefig(stdout,format='png')

def main1():
    #data=load(open('ex.json'))
    data=load(stdin)
    plot2(data)

if __name__=='__main__':
    main1()
