Our job here is to implement a program (using the pthread library) based on the idea of the following pseudocode:

main thread:
create Ν workers
while (job exists) {
  wait for a worker to become available
  read next value & assign it to an available worker
  notify the worker to process the value
}
wait for all workers to become available
notify workers to terminate
wait for all workers to terminate

worker thread:
while (1) {
  notify main that I am available
  wait for notification by main
  if notified to terminate, break
  else process assigned value
}
notify main that I will terminate
