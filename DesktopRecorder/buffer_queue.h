/* *
 * Author:     dengjinhui <dengjinhui@cdzs.com>
 *
 * Maintainer: dengjinhui <dengjinhui@cdzs.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */

#ifndef BUFFER_QUEUE_H
#define BUFFER_QUEUE_H

#include <iostream>
extern "C" {

	//单链表,无拷贝队列
	struct Node_t {
		uint8_t** _data = nullptr;
		Node_t* next = nullptr;
		Node_t() {

		}
		Node_t(uint8_t* data, size_t len) {
			if (data) {
				_data = &data;
			}
		}
		~Node_t() {
			_data = nullptr;
			next = nullptr;
		}
	};


	struct Buffer_Queue {
		static inline void releaseNode(Node_t* node) {
			if (node->next)
				releaseNode(node->next);
			delete node;
		}

		void setCapacity(int c) { capacity = c; }
		void releaseBuffer() {
			Node_t* p = head->next;
			uint32_t i = 0;
			releaseNode(p);
			buffer_size = 0;
			head->next = nullptr;
			delete head;
			head = new Node_t;
			tail = nullptr;
			currentNum = 0;
		}
		int getCurrent() { return currentNum; }
		Buffer_Queue() {
			head = new Node_t;
			tail = nullptr;
		}

		Node_t* head;
		Node_t* tail;
		uint32_t currentNum = 0;
		uint32_t capacity = 10;
		size_t buffer_size = 0;
	};

	static inline bool put_buffer(Buffer_Queue* queue, uint8_t* data, size_t len) {
		if (queue->buffer_size == 0) {
			queue->buffer_size = len;
		}
		else if (queue->buffer_size != len) {
			printf("buffer size different!\n");
			return false;
		}
		if (queue->currentNum + 1 >= queue->capacity) {
			printf("buffer_queue full\n");
			return false;
		}
		Node_t* node = new Node_t(data, len);
		if (queue->tail) {
			queue->tail->next = node;
		}
		else {
			queue->head->next = node;
		}
		queue->tail = node;
		queue->currentNum++;
		return true;
	}

	static inline bool get_buffer(Buffer_Queue* queue, uint8_t*& dst) {
		Node_t* p = queue->head->next;
		if (!p) return false;
		dst = *(p->_data);
		queue->head->next = p->next;
		if (p->next == nullptr)
			queue->tail = nullptr;
		queue->currentNum--;
		return true;
	}

}



#endif // BUFFER_QUEUE_H
