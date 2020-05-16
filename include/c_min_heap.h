#ifndef _C_MIN_HEAP_H_
#define _C_MIN_HEAP_H_

#define lpHeapNode struct HeapNode*

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

using std::vector;

template<class T>
class CMinHeap
{
	public:
		CMinHeap(){
			m_root = nullptr;
			m_dir = 0;
		};
		CMinHeap(T _k ){
			m_root = new struct HeapNode(_k);
		};
		~CMinHeap();

	public:
		struct HeapNode
		{
			struct HeapNode* left;
			struct HeapNode* right;
			T key;
			HeapNode(T _key):left(nullptr),right(nullptr),key(_key){};
		};

	public:
		void pushNode(T _k); //外部调用时传nullptr即可
		void dropNode();
		void showTree(); //打印树图
		int calHeight();
		int checkRoot();
		T getRoot();

	private:
		void dropNode(lpHeapNode& _root);
		void pushNode(lpHeapNode _new_node,lpHeapNode& _root);
		void showTree(lpHeapNode _root, int step, int height, int h_pos, char left, int** _map);

	private:
		void deleteNode(lpHeapNode _root);
		int calHeight(lpHeapNode _root,int _step);

	private:
		lpHeapNode m_root;
		char m_dir:1;
};

template<typename T>
CMinHeap<T>::~CMinHeap(){
	deleteNode(m_root);
}

template<typename T>
void CMinHeap<T>::dropNode()
{
	dropNode(m_root);
}

template<typename T>
void CMinHeap<T>::dropNode(lpHeapNode& _root)
{
	if(_root)
	{
		if(_root->left && _root->right)
		{
			if(_root->left->key < _root->right->key)
			{
				_root->key = _root->left->key;
				return dropNode(_root->left);
			}
			else
			{
				_root->key = _root->right->key;
				return dropNode(_root->right);
			}
		}
		else if(_root->left)
		{
			_root->key = _root->left->key;
			return dropNode(_root->left);
		}
		else if(_root->right)
		{
			_root->key = _root->right->key;
			return dropNode(_root->right);
		}
		else
		{
			//叶子节点
			delete _root;
			_root = nullptr;
		}
	}
}

template<typename T>
void CMinHeap<T>::pushNode(T _k)
{
	lpHeapNode new_node = new struct HeapNode(_k);
	pushNode(new_node, m_root);
	m_dir = ~m_dir;
}

/*插入key
 *1.找到第一个比要插入的key大的节点 a
 *2.将新节点和a的数据交换 这样我们不用处理 a的父节点的指向
 *3.将新节点设置为a的子节点
 * */
template<typename T>
void CMinHeap<T>::pushNode(lpHeapNode _new_node,lpHeapNode& _root)
{
	if(!_root)
	{
		_root = _new_node;
		return;
	}
	if(m_dir == 0) //left begin
	{
		if(_new_node->key < _root->key)
		{
			T temp_key = _new_node->key;
			*_new_node = *_root;
			_root->key = temp_key;
			_root->left = _new_node;
			_new_node->right = nullptr;
			//_root->right = nullptr;
			return;
		}
		pushNode(_new_node,_root->left);
	}
	else
	{
		if(_new_node->key < _root->key)
		{
			T temp_key = _new_node->key;
			*_new_node = *_root;
			_root->key = temp_key;
			_root->right = _new_node;
			_new_node->left = nullptr;
			//_root->left = nullptr;
			return;
		}
		pushNode(_new_node,_root->right);
	}
}

//析构函数调用
template<typename T>
void CMinHeap<T>::deleteNode(lpHeapNode _root)
{
	if(_root)
	{
		deleteNode(_root->left);
		deleteNode(_root->right);
		delete _root;
	}
}

template<typename T>
void CMinHeap<T>::showTree()
{
	if(!m_root)
	{
		std::cout<<"    empty tree   "<<std::endl;
		return;
	}
	int height = calHeight();
	//int map[height][height*2 - 1]{-1,};
	int **map = new int*[height];
	int i,j;
	int v_len = pow(2,height) - 1;
	for(i = 0; i < height; i++)
	{
		map[i] = new int[v_len];
		for(j = 0; j<v_len; j++)
			map[i][j] = -1;
	}
	//memset(map,-1,height*(height*2 - 1));
	map[0][v_len / 2] = m_root->key;
	showTree(m_root->left, 0, height, v_len / 2, 1, map);
	showTree(m_root->right, 0, height, v_len / 2, 0, map);
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < v_len; j++)
		{
			if(map[i][j] == -1)
			{
				std::cout<<" ";
			}
			else
			{
				std::cout<<map[i][j];
			}
		}
		std::cout<<std::endl;
	}
}

template<typename T>
void CMinHeap<T>::showTree(lpHeapNode _root, int step, int height, int h_pos, char left, int** map)
{
	if(!_root)
		return;
	step++;
	int diff = pow(2,height - 1 - step);
	if(left)
		h_pos -= diff;
	else
		h_pos += diff;
	map[step][h_pos] = _root->key;
	std::cout<<"s: "<<step<<", h: "<<height<<", pos: "<<h_pos<<", diff: "<<diff<<std::endl;
	showTree(_root->left, step, height, h_pos, 1, map);
	showTree(_root->right, step, height, h_pos, 0, map);
}

template<typename T>
int CMinHeap<T>::calHeight()
{
	return calHeight(m_root,1);
}

template<typename T>
int CMinHeap<T>::calHeight(lpHeapNode _root,int _step)
{
	if(!_root || (!_root->left && !_root->right))
	{
		return _step;
	}
	_step++;
	int left_step = calHeight(_root->left,_step);
	int right_step = calHeight(_root->right,_step);
	return left_step > right_step? left_step:right_step;
}

template<typename T>
int CMinHeap<T>::checkRoot()
{
	return m_root? 0:1;
}

template<typename T>
T CMinHeap<T>::getRoot()
{
	return m_root->key;
}

#endif
