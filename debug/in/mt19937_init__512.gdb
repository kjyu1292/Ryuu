set width 0
set height 0
set verbose off

# Setting breaks
break _mt19937_simd.s:281
break _mt19937_simd.s:291
break _mt19937_simd.s:294
break _mt19937_simd.s:297
break _mt19937_simd.s:300
break _mt19937_simd.s:306

# Start
run

# 283
step
print /u $zmm0.v16_int32
print /u $zmm2.v16_int32
print /u $zmm3.v16_int32
print /u $zmm4.v16_int32

# 293
continue
step
print /u $zmm1.v16_int32

# 296
step
print /u $zmm0.v16_int32

# 299
step
print /u $zmm0.v16_int32

# 302
step
print /u $zmm0.v16_int32

# 308
continue
step
print /u $zmm4.v16_int32

set $i = 1
while $i < 1000
	# 293
	continue
	step
	print /u $zmm1.v16_int32
	
	# 296
	step
	print /u $zmm0.v16_int32
	
	# 299
	step
	print /u $zmm0.v16_int32
	
	# 302
	step
	print /u $zmm0.v16_int32
	
	# 308
	continue
	step
	print /u $zmm4.v16_int32

	set $i = $i + 1
end

# End
echo \n \n
quit

