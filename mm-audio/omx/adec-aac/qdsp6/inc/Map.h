/*--------------------------------------------------------------------------
Copyright (c) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
--------------------------------------------------------------------------*/
#ifndef _MAP_H_
#define _MAP_H_

#include <stdio.h>
using namespace std;

template <typename T,typename T2>
class Map
{
    struct node
    {
        T    data;
        T2   data2;
        node* prev;
        node* next;
        node(T t, T2 t2,node* p, node* n) :
             data(t), data2(t2), prev(p), next(n) {}
    };
    node* head;
    node* tail;
    node* tmp;
    unsigned size_of_list;
    static Map<T,T2> *m_self;
public:
    Map() : head( NULL ), tail ( NULL ),tmp(head),size_of_list(0) {}
    bool empty() const { return ( !head || !tail ); }
    operator bool() const { return !empty(); }
    void insert(T,T2);
    void show();
    int  size();
    T  get_head();
    T2 find(T); // Return VALUE
    T find_ele(T);// Check if the KEY is present or not
    T2 begin(); //give the first ele
    bool erase(T);
    bool eraseall();
    bool isempty();
    ~Map()
    {
        while(head)
        {
            node* temp(head);
            head=head->next;
            size_of_list--;
            delete temp;
        }
    }
};

template <typename T,typename T2>
T2 Map<T,T2>::find(T d1)
{
    tmp = head;
    while(tmp)
    {
        if(tmp->data == d1)
        {
            return tmp->data2;
        }
        tmp = tmp->next;
    }
    return 0;
}

template <typename T,typename T2>
T Map<T,T2>::find_ele(T d1)
{
    tmp = head;
    while(tmp)
    {
        if(tmp->data == d1)
        {
            return tmp->data;
        }
        tmp = tmp->next;
    }
    return 0;
}

template <typename T,typename T2>
T2 Map<T,T2>::begin()
{
    tmp = head;
    if(tmp)
    {
        return (tmp->data2);
    }
    return 0;
}

template <typename T,typename T2>
void Map<T,T2>::show()
{
    tmp = head;
    while(tmp)
    {
        DEBUG_PRINT("%d-->%d\n",tmp->data,tmp->data2);
        tmp = tmp->next;
    }
}

template <typename T,typename T2>
int Map<T,T2>::size()
{
    int count =0;
    tmp = head;
    while(tmp)
    {
        tmp = tmp->next;
        count++;
    }
    return count;
}

template <typename T,typename T2>
void Map<T,T2>::insert(T data, T2 data2)
{
    tail = new node(data, data2,tail, NULL);
    if( tail->prev )
        tail->prev->next = tail;

    if( empty() )
    {
        head = tail;
        tmp=head;
    }
    tmp = head;
    size_of_list++;
}

template <typename T,typename T2>
bool Map<T,T2>::erase(T d)
{
    bool found = false;
    tmp = head;
    node* prevnode = tmp;
    node *tempnode;

    while(tmp)
    {
        if((head == tail) && (head->data == d))
        {
           found = true;
           tempnode = head;
           head = tail = NULL;
           delete tempnode;
           break;
        }
        if((tmp ==head) && (tmp->data ==d))
        {
            found = true;
            tempnode = tmp;
            tmp = tmp->next;
            tmp->prev = NULL;
            head = tmp;
            tempnode->next = NULL;
            delete tempnode;
            break;
        }
        if((tmp == tail) && (tmp->data ==d))
        {
            found = true;
            tempnode = tmp;
            prevnode->next = NULL;
            tmp->prev = NULL;
            tail = prevnode;
            delete tempnode;
            break;
        }
        if(tmp->data == d)
        {
            found = true;
            prevnode->next = tmp->next;
            tmp->next->prev = prevnode->next;
            tempnode = tmp;
            delete tempnode;
            break;
        }
        prevnode = tmp;
        tmp = tmp->next;
    }
    if(found)size_of_list--;
    return found;
}

template <typename T,typename T2>
bool Map<T,T2>::eraseall()
{
    node *tempnode;
    tmp = head;
    while(head)
    {
       tempnode = head;
       tempnode->next = NULL;
       head = head->next;
       delete tempnode;
    }
    tail = head = NULL;
    return true;
}


template <typename T,typename T2>
bool Map<T,T2>::isempty()
{
    if(!size_of_list) return true;
    else return false;
}

template <typename T,typename T2>
T Map<T,T2>::get_head()
{
    tmp = head;
    if(tmp)
    {
        return tmp->data;
    }
    return NULL;
}

#endif // _MAP_H_
