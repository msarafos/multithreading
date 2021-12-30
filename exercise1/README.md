Our job here is to implement a program (using the pthread library) based on the idea of the following pseudocode:

main thread: <br />
create Ν workers <br />
while (job exists) { <br />
    wait for a worker to become available <br />
    read next value & assign it to an available worker <br /> 
    notify the worker to process the value <br />
} <br />
wait for all workers to become available <br />
notify workers to terminate <br />
wait for all workers to terminate <br />

worker thread: <br />
while (1) { <br />
    notify main that I am available <br />
    wait for notification by main <br />
    if notified to terminate, break <br />
    else process assigned value <br />
} <br />
notify main that I will terminate <br />
