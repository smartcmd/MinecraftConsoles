#include "stdafx.h"
#include "Node.h"
#include "System.h"
#include "BasicTypeContainers.h"
#include "BinaryHeap.h"

// 4J Jev, add common ctor code.
void BinaryHeap::_init()
{
	heap = NodeArray(1024);
	sizeVar = 0;
}

BinaryHeap::BinaryHeap()
{
	_init();
}

BinaryHeap::~BinaryHeap()
{
	delete[] heap.data;
}

Node *BinaryHeap::insert(Node *node)
{
    /* if (node->heapIdx >=0) throw new IllegalStateException("OW KNOWS!"); 4J Jev, removed try/catch */

    // Expand if necessary.
	if (sizeVar == heap.length)
    {
        NodeArray newHeap = NodeArray(sizeVar << 1);

        System::arraycopy(heap, 0, &newHeap, 0, sizeVar);

		delete[] heap.data;
        heap = newHeap;
    }

    // Insert at end and bubble up.
    heap[sizeVar] = node;
    node->heapIdx = sizeVar;
    upHeap(sizeVar++);

    return node;
}

void BinaryHeap::clear()
{
    sizeVar = 0;
}

Node *BinaryHeap::peek()
{
    return heap[0];
}

Node *BinaryHeap::pop()
{
    Node *popped = heap[0];
    heap[0] = heap[--sizeVar];
    heap[sizeVar] = nullptr;
    if (sizeVar > 0) downHeap(0);
    popped->heapIdx=-1;
    return popped;
}

void BinaryHeap::remove(Node *node)
{
    // This is what node.heapIdx is for.
    heap[node->heapIdx] = heap[--sizeVar];
    heap[sizeVar] = nullptr;
    if (sizeVar > node->heapIdx)
    {
        if (heap[node->heapIdx]->f < node->f)
        {
            upHeap(node->heapIdx);
        }
        else
        {
            downHeap(node->heapIdx);
        }
    }
    // Just as a precaution: should make stuff blow up if the node is abused.
    node->heapIdx = -1;
}

void BinaryHeap::changeCost(Node *node, float newCost)
{
    float oldCost = node->f;
    node->f = newCost;
    if (newCost < oldCost)
    {
        upHeap(node->heapIdx);
    }
    else
    {
        downHeap(node->heapIdx);
    }
}

int BinaryHeap::size()
{
    return sizeVar;
}

void BinaryHeap::upHeap(int idx)
{
    Node *node = heap[idx];
    float cost = node->f;
    while (idx > 0)
    {
        int parentIdx = (idx - 1) >> 1;
        Node *parent = heap[parentIdx];
        if (cost < parent->f)
        {
            heap[idx] = parent;
            parent->heapIdx = idx;
            idx = parentIdx;
        }
        else break;
    }
    heap[idx] = node;
    node->heapIdx = idx;
}

void BinaryHeap::downHeap(int idx)
{
    Node *node = heap[idx];
    float cost = node->f;

    while (true)
    {
        int leftIdx = 1 + (idx << 1);
        int rightIdx = leftIdx + 1;

        if (leftIdx >= sizeVar) break;

        // We definitely have a left child.
        Node *leftNode = heap[leftIdx];
        float leftCost = leftNode->f;
        // We may have a right child.
        Node *rightNode;
        float rightCost;

        if (rightIdx >= sizeVar)
        {
            // Only need to compare with left.
            rightNode = nullptr;
            rightCost = Float::POSITIVE_INFINITY;
        }
        else
        {
            rightNode = heap[rightIdx];
            rightCost = rightNode->f;
        }

        // Find the smallest of the three costs: the corresponding node
        // should be the parent.
        if (leftCost < rightCost)
        {
            if (leftCost < cost)
            {
                heap[idx] = leftNode;
                leftNode->heapIdx = idx;
                idx = leftIdx;
            }
            else break;
        }
        else
        {
            if (rightCost < cost)
            {
                heap[idx] = rightNode;
                rightNode->heapIdx = idx;
                idx = rightIdx;
            }
            else break;
        }
    }

    heap[idx] = node;
    node->heapIdx = idx;
}

bool BinaryHeap::isEmpty()
{
    return sizeVar==0;
}