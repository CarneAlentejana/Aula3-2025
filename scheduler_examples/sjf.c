#include "sjf.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    if (*cpu_task == NULL) {
        queue_elem_t *elem_atual = rq->head;
        queue_elem_t *menor_elem = NULL;
        pcb_t *menor = NULL;
        //se elem_atual for NULL, a fila está vazia
        while (elem_atual != NULL) {
            if (!menor || elem_atual->pcb->time_ms < menor->time_ms) {
                menor = elem_atual->pcb;
                menor_elem = elem_atual;
            }
            elem_atual = elem_atual->next;
        }
        if (menor_elem != NULL) {
            queue_elem_t *removed = remove_queue_elem(rq, menor_elem);
            *cpu_task = removed->pcb;
            free(removed);
        }
    }
}