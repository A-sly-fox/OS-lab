.global main
main:
	li	a0, 1
	li	a1, 100
	li	a2, 0
	bgt	a0, a1, end # if a0 > a1 then end
	
loop:
	add	a2, a0, a2; # a2 = a0 + a2
	addi	a0, a0, 1; # a0 = a0 + 1
	ble	a0, a1, loop
	
end:
	j	end
