This DFU bootloader lives in the upper portion of memory and executes entirely from RAM.

The first thing that it does on start is edit the startup vector table to redirect to itself - this way, if a partial firmware image that can't start the bootloader is uploaded, the bootloader doesn't get locked out. The last thing it does before concluding is populating the startup vector table.
