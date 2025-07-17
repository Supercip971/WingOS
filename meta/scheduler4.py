
import time
import math
import random
import threading

tid = 0
class CpuTreeNode:
    def __init__(self, cpu_id, children=None, is_leaf=False):
        self.cpu_id = cpu_id
        self.children = children or []
        self.is_leaf = is_leaf

    def total_weight(self):
        if self.is_leaf:
            return 1
        return sum(child.total_weight() for child in self.children)
   
    def distance_to(self, other):
        """Return the distance between this node and other, or None if not a descendant."""
        if self == other:
            return 0
        
        for child in self.children:
            if child == other:
                return 1
            d = child.distance_to(other)
            if d is not None:
                return 1 + d

        return None
    
    def are_they_siblings(self, a, b):
        """
        Return True if nodes a and b are siblings in the tree.
        This method searches recursively in the children.
        """

        if a in self.children and b in self.children:
            return True
        
        # First, check among our immediate children.
        for child in self.children:
            if a in child.children and b in child.children:
                return True
        # Recursively check among deeper levels.
        for child in self.children:
            if child.are_they_siblings(a, b):
                return True
        return False
    
    def __repr__(self):
        return f"CPU({self.cpu_id})"
    
    def count(self):
        if self.is_leaf:
            return 1
        return sum(child.count() for child in self.children)

class Task:
    def __init__(self, name, base_priority, nice=0):
        self.name = name
        self.base_priority = base_priority
        self.nice = nice
        self.running = 0
        self.sleeping = 0
        self.old_cpu = None
        self.cpu_affinity = None
        self.total_cycles = 0

    def __repr__(self):
        return f"{self.name} (prio: {self.depriority()}, runs: {self.running})"
    
    def tick(self, t):
        #self.running += t
        self.total_cycles += 1

    # Lower values are considered better.
    def depriority(self):
        return self.base_priority + self.running - self.sleeping

class MultiCPUScheduler:
    def __init__(self, cpu_tree, total_cycles=20):
        self.cpu_tree = cpu_tree
        # Create 128 task queues (each index represents a priority level)

        self.task_queues = [[] for _ in range(128)]
        self.total_cycles = total_cycles
        self.current_cycle = 0
        self.runned_tasks = []
        self.cpu_runned = {}

    def task_queue_id(self, task):
        # Ensure the task is placed within the 0-127 range.
        return min(max(0, task.depriority() - 1), 127)
    
    def push_task_first(self, task):
        global tid
        task.name = f"Task-{tid}"
        tid += 1
        
        #self.cpu_runned[0] = task
        #self.runned_tasks.append(task)
        qid = self.task_queue_id(task)
        self.task_queues[qid].append(task)
        #print(f"Added task: {task} to queue {qid}")
    def add_task(self, task):
        qid = self.task_queue_id(task)
        self.task_queues[qid].append(task)
        #print(f"Added task: {task} to queue {qid}")
    
    def task_count(self):
        return sum(len(q) for q in self.task_queues) + len(self.cpu_runned.values())

    def cpu_count(self):
        i = 0
        for cpu in self.cpu_tree.children:
            i += cpu.count()
        return i

    def query_shortest_path_id(self, task_queue, cpu, consider_sibling=False):
        """
        For tasks in the given queue, choose the one whose CPU affinity is either unset,
        already matching the given CPU, or whose affinity is “close” (i.e. sibling) to the given CPU.
        """
        best_i = None
        for i, task in enumerate(task_queue):
            if task.cpu_affinity is None:
                # First time scheduling: assign this CPU as affinity.
                task.cpu_affinity = cpu
                return i
            if task.cpu_affinity == cpu:
                return i
            # Check if the task's CPU affinity and the current cpu are siblings in the CPU tree.
            if consider_sibling and self.cpu_tree.are_they_siblings(task.cpu_affinity, cpu):
                best_i = i
        return best_i
    
    def update_runned_tasks(self):
        # move each queue from 2 -> 1 and 3 -> 2 and 4 -> 3...

        for i in range(0, len(self.task_queues)-1):
            if i == 0:
                self.task_queues[i] = self.task_queues[i] + self.task_queues[i+1]
            else: 
                self.task_queues[i] = self.task_queues[i+1]
        self.task_queues[len(self.task_queues)-1] = []
        c = self.task_count()


#        tick_cost = max(2 * self.cpu_count() - c + 1, 1)
        avg_priority = 0
        for queue in self.task_queues:
            for task in queue:
                avg_priority += task.depriority()
        

        for queue in self.task_queues:
            for task in queue:
                # T = C+n
                # d = C-n+1
                # => d = 2C-T+1
                task.sleeping += math.ceil(avg_priority / c) + task.base_priority
               # task.sleeping +=max(2*self.cpu_count()-c+1, 1)

        for key, task in self.cpu_runned.items():
            task.running += 1
            
            task.old_cpu = key
            self.add_task(task)
        
       
        self.cpu_runned = {}
        #while self.cpu_runned != {}:
        #    task = self.cpu_runned.pop()
        #    task.running += 1
        #    #task.running -= max((c -1)* self.cpu_count(), 1)
        #    self.add_task(task)

#        while self.runned_tasks != []:
#            task = self.runned_tasks.pop(0)
#            task.running += 1
#            #task.running -= max((c -1)* self.cpu_count(), 1)
#            self.add_task(task)    

    def fix_affinity(self): 


        to_fix = [v for v in self.cpu_runned.keys()]

        while to_fix != []:
            cpu = to_fix.pop()
            task = self.cpu_runned[cpu]
      #  for (cpu, task) in (self.cpu_runned.items()):

            # Ok so the task will be moved from cpu -> old_cpu 
            # the swapped one will be moved to old_cpu -> cpu
            old_cpu = task.old_cpu 
           # cpu = task.cpu_affinity
            if(old_cpu != None and cpu != old_cpu): # we jumped to another cpu
              #  print(f"task {task.name} swapped {old_cpu}->{cpu}")
                # if the other task was not affiliated with her new cpu, we swap it 
                stealer_task = self.cpu_runned.get(old_cpu)

                if stealer_task != None  and (stealer_task.cpu_affinity != cpu or task.depriority() < stealer_task.depriority()): 
                    #stealer_task.cpu_affinity = old_cpu
                    #self.add_task(stealer_task)


                    #self.runned_tasks.remove(stealer_task)
                 #   print(f"Stealing {stealer_task} from {old_cpu} to {cpu}")
                    stealer_task.cpu_affinity = cpu
                    self.cpu_runned[cpu] = stealer_task

                    task.cpu_affinity = old_cpu
                    self.cpu_runned[old_cpu] = task
                    to_fix.append((old_cpu))
                else:
                    pass
                  #  print(f"Stealing {stealer_task} from {stealer_task.old_cpu} to {old_cpu}")
#                    self.runned_tasks.append(task)
                   # self.run_task_queued(old_cpu, queue_id, queue_offset,c)
        
    def run_task_queued(self, cpu, queue_id, queue_offset,c):
        task = self.task_queues[queue_id].pop(queue_offset)
        #if(self.cpu_runned[cpu] != task):
            

        task.tick(1)
        task.cpu_affinity = cpu
        #self.runned_tasks.append(task)
        self.cpu_runned[cpu] = task 

        #print(f"| (cpu:{cpu}) {task} ", end="")
    
    def tick_all(self):
        c = self.task_count()
   
        choosen = [i for i in range(0, self.cpu_count())]
        for (i, task_queue) in enumerate(self.task_queues):
            retried = []
            while len(choosen) > 0:
                cpu = choosen.pop()
                task_index = self.query_shortest_path_id(task_queue, cpu)
                if task_index is not None:
                    self.run_task_queued(cpu, i, task_index,c)
                #    choosen.remove(cpu)
                else :
                    retried.append(cpu)
            

            while len(task_queue) > 0 and len(retried) > 0:
                cpu = retried.pop()
                self.run_task_queued(cpu, i, 0,c)
            
            if len(retried) == 0:
                break

            choosen = retried
        
        
        

    def tick_force(self, cpu):
        """
        Go over the task queues in order (lowest indices = highest priority)
        and pick one task for the given CPU to run.
        """

        c = self.task_count()
        for (i,task_queue) in enumerate(self.task_queues):
            if not task_queue:
                continue

            task_index = self.query_shortest_path_id(task_queue, cpu)
            # If none of the tasks in this queue match the affinity criteria, fallback to the first.
            if task_index is None:
              #  if self.task_queues[i+1] != []:
              #      sub_task_index = self.query_shortest_path_id(self.task_queues[i+1], cpu, False)
              #      if sub_task_index is not None:
              #          self.run_task_queued(cpu, i+1, sub_task_index,c)
              #          return
                task_index = 0
            # Run only one task per tick.

            self.run_task_queued(cpu, i, task_index,c)
            return
        print(f"Cycle {self.current_cycle}: {cpu} found no task to run.")
    
    def simulate(self, cpus):
        """
        Simulate a number of scheduling cycles across the provided CPU nodes.
        """
        while self.current_cycle < self.total_cycles:
            self.current_cycle += 1
            self.tick_all()
       #     for cpu in cpus:
       #         self.tick_force(cpu)
            self.fix_affinity()


            cl = [None for _ in range(self.cpu_count())]
            for (v,c) in self.cpu_runned.items():
                cl[v] = c 
            for c in cl:
                print(f"| {c} ", end="")
            self.update_runned_tasks()
            self.cpu_runned = {}
            print("")
          #  time.sleep(0.02)

def build_cpu_tree():
    """
    Build a simple CPU tree:
      - One root node with 4 children (leaf nodes representing 4 CPUs).
    """
    leaf00 = CpuTreeNode(cpu_id=0, is_leaf=True)
    leaf01 = CpuTreeNode(cpu_id=1, is_leaf=True)
    leaf10 = CpuTreeNode(cpu_id=2, is_leaf=True)
    leaf11 = CpuTreeNode(cpu_id=3, is_leaf=True)
    root1 = CpuTreeNode(cpu_id=-1, children=[leaf00, leaf01], is_leaf=False)
    root2 = CpuTreeNode(cpu_id=-2, children=[leaf10, leaf11], is_leaf=False)
    root = CpuTreeNode(cpu_id=-3, children=[root1, root2], is_leaf=False)
    return root, [leaf00, leaf01, leaf10, leaf11]

def main():
    # Build our CPU tree and get the leaf CPUs.
    cpu_tree, cpus = build_cpu_tree()

    # Create the scheduler with the CPU tree and a total number of cycles.
    scheduler = MultiCPUScheduler(cpu_tree, total_cycles=512)

    # Create some tasks with random base priorities.
    tasks = [Task(f"Task{i}", base_priority=1) for i in range(4)]
    tasks.append(Task("Task{65}", base_priority=3))
    for task in tasks:
        scheduler.push_task_first(task)
    
    # Run the simulation over the CPUs.
    scheduler.simulate(cpus)

    for t in scheduler.task_queues:
        for task in t:
            print(f"Task: {task.name} ran {task.total_cycles} cycles.")
    for task in scheduler.cpu_runned.values():
        print(f"Task: {task.name} ran {task.total_cycles} cycles.")

#    for task in tasks:
#        print(f"Task {task.name} ran {task.total_cycles} cycles.")

    print("\nSimulation complete. The tasks that ran:")
    for task in scheduler.cpu_runned.values():
        print(task)

if __name__ == '__main__':
    main()



