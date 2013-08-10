
paintMCM
		push ebp
		mov esp,ebp
		
		mov edx,[_bitpattern]
		mov edi,[_gcol]
		mov esi,[_mnshline]
		
		mov eax,edx
		and eax,0xc0
		shl eax,6
		mov ebx,[eax+_mcmcolors]
		mov ecx,[eax+priority]
		mov [esi],ebx
		mov [edi],ecx
		
		mov eax,edx
		and eax,0x30
		shl eax,4
		mov ebx,[eax+_mcmcolors]
		mov ecx,[eax+priority]
		mov [esi+2],ebx
		mov [edi+2],ecx

		mov eax,edx
		and eax,0x0c
		shl eax,2
		mov ebx,[eax+_mcmcolors]
		mov ecx,[eax+priority]
		mov [esi+4],ebx
		mov [edi+4],ecx
		
		mov eax,edx
		and eax,0x03
		mov ebx,[eax+_mcmcolors]
		mov ecx,[eax+priority]
		mov [esi+6],ebx
		mov [edi+6],ecx
		
		add [_mnshline],8
		add [_gcol],8
		mov esp,ebp
		pop ebp
		ret