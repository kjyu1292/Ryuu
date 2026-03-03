set width 0
set height 0
set verbose off

# Setting breaks
break _mt19937_simd.s:360
break _mt19937_simd.s:374
break _mt19937_simd.s:376
break _mt19937_simd.s:387
break _mt19937_simd.s:399
break _mt19937_simd.s:405
break _mt19937_simd.s:435

# Start
run

# 362
step
print /u $rbx
print /u $rax

# 376
continue
step
print /u $zmm0.v16_int32
print /u $zmm1.v16_int32

# 378
step
print /u $zmm0.v16_int32

# 389
continue
step
print /u $zmm1.v16_int32

# 401
continue
step
print /u $rax

# 407
continue
step
print /u $zmm0.v16_int32

# 437
continue
step
print /u $zmm0.v16_int32

set $i = 1
while $i < 1000
	# 362
	continue
	step
	print /u $rbx
	print /u $rax
	
	# 376
	continue
	step
	print /u $zmm0.v16_int32
	print /u $zmm1.v16_int32
	
	# 378
	step
	print /u $zmm0.v16_int32
	
	# 389
	continue
	step
	print /u $zmm1.v16_int32
	
	# 401
	continue
	step
	print /u $rax
	
	# 407
	continue
	step
	print /u $zmm0.v16_int32
	
	# 437
	continue
	step
	print /u $zmm0.v16_int32

	set $i = $i + 1
end

# End
echo \n \n
quit

