README.txt


Part 1


Assumptions
1) The instruction does not say how to handle errors; we handle only the case where the user enters less than 11 characters (the minimum for a successful call)
We do not handle errors further, for exampe if the user tried to enter 0000000000000 our program would seg fault
This was acceptable for error handling because we could give them recourse for obvious mistakes, however we do not use regular expressions to fine-tune our error handling. I feel the scope of this project, just assuming users will enter proper arguments is reasonable

2) We follow the style used within the instructions
We can, for example, enter 0,map,0,1 but we do not look to handle 0, map, 0, 1
Just seemed unnecessary for this project - know we cannot handle spaces well

3) We are submitting only part 2, as forum posts allows us to do- there will be no part 1 submission

4) The way we handle swapping is
	- We store values to "file" as soon as they are stored to memory
	- When we remove a page from memory, we just delete it
	- Page is already stored correctly
	
