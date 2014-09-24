/*
 * Copyright 2005 Paul Walker
 *
 * GNU General Public License
 * 
 * This file is part of: Kernel Emulation on Windows (keow)
 *
 * Keow is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Keow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keow; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//Simple List implementation (like STL) (see comment in includes.h)
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIST_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)
#define AFX_LIST_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


template<typename T>
class list
{
public:
	class iterator;
protected:
	class node
	{
		friend list;
		friend iterator;

		node(T val)
			: value(val)
			, next(0), prev(0)
		{
		}

		node *prev, *next;
		T value;
	};
	node *head, *tail;

public:
	class iterator
	{
	private:
		friend list;
		node *n;
	public:
		iterator()
		{
			n=0;
		}

		T& operator*()
		{
			return n->value;
		}

		iterator& operator=(iterator& other)
		{
			n = other.n;
			return *this;
		}

		bool operator==(iterator& other)
		{
			return n == other.n;
		}

		bool operator!=(iterator& other)
		{
			return n != other.n;
		}

		void operator++()
		{
			if(n!=0)
			{
				n = n->next;
			}
		}
	};

public:
	list();
	~list();

	list& operator=(const list& other);
	typename T& operator[](int index);

	list& push_back(typename T element);
	typename T pop_back();
	iterator find(typename T value);
	void erase(iterator it);
	void erase(typename T value);
	void clear();
	bool empty() const;
	int size() const;

	iterator begin() const;
	iterator end() const;
};


/////////////////////////////////////////////////////////////////


template<typename T> inline
list<T>::list()
{
	head=tail=NULL;
}

template<typename T> inline
list<T>::~list()
{
	clear();
}

template<typename T> inline
list<T>& list<T>::operator=(const list<typename T>& other)
{
	clear();

	list<T>::iterator it;
	for(it=other.begin(); it!=other.end(); ++it)
	{
		push_back( *it );
	}
	return *this;
}

template<typename T> inline
T& list<T>::operator[](int index)
{
	list<T>::iterator it;
	for(it=begin(); it!=end(); ++it)
	{
		if(index==0)
			break;
		--index;
	}
	return *it;
}

template<typename T> inline
list<T>& list<T>::push_back(T element)
{
	list<T>::node *n = new list<T>::node(element);
	n->next = NULL;
	n->prev = tail;
	if(tail) //already have a tail?
		tail->next=n; //add to end of list
	else
		head = n; //first element in list
	tail = n; //new tail

	return *this;
}

template<typename T> inline
T list<T>::pop_back()
{
	list<T>::node *n = tail; //save

	tail = tail->prev; //move tail back one
	if(tail)
		tail->next = NULL; //new tail is end
	else
		head=NULL; //removed entire list


	T value = n->value;
	delete n;
	return value;
}

template<typename T> inline
typename list<T>::iterator list<T>::find(typename T value)
{
	typename list<T>::iterator it;
	for(it=begin(); it!=end(); ++it)
	{
		if(it.n->value == value)
			return it;
	}
	return it;
}

template<typename T> inline
void list<T>::erase(typename list<T>::iterator it)
{
	if(it.n==NULL)
		return;

	list<T>::node *n = it.n;
	if(n->prev)
		n->prev->next = n->next;
	if(n->next)
		n->next->prev = n->prev;

	if(head==n)
		head=n->next;
	if(tail==n)
		tail=n->prev;

	delete n;
}

template<typename T> inline
void list<T>::erase(typename T value)
{
	list<T>::erase(list<T>::find(value));
}

template<typename T> inline
void list<T>::clear()
{
	while(!empty())
		pop_back();
}

template<typename T> inline
bool list<T>::empty() const
{
	return head==NULL;
}

template<typename T> inline
int list<T>::size() const
{
	int cnt = 0;
	typename list<T>::iterator it;
	for(it=begin(); it!=end(); ++it)
	{
		++cnt;
	}
	return cnt;
}


template<typename T> inline
typename list<T>::iterator list<T>::begin() const
{
	typename list<T>::iterator it;
	it.n = head;
	return it;
}

template<typename T> inline
typename list<T>::iterator list<T>::end() const
{
	typename list<T>::iterator it;
	it.n = NULL;
	return it;
}


#endif // !defined(AFX_LIST_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)
