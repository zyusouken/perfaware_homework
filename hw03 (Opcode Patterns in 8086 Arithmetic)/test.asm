bits 16
mov si, bx
mov dh, al
mov cl, 12
mov ch, -12
mov cx, 12
mov cx, -12
mov dx, 3948
mov dx, -3948
mov al, [bx + si]
mov bx, [bp + di]
mov dx, [bp]
mov ah, [bx + si + 4]
mov al, [bx + si + 4999]
mov [bx + di], cx
mov [bp + si], cl
mov [bp], ch
mov ax, [bx + di - 37]
mov [si - 300], cx
mov dx, [bx - 32]
mov [bp + di], byte 7
mov [di + 901], word 347
mov bp, [5]
mov bx, [3458]
mov ax, [2555]
mov ax, [16]
mov [2554], ax
mov [15], ax
push word [bp + si]
push word [3000]
push word [bx + di - 30]
push cx
push ax
push dx
push cs
pop word [bp + si]
pop word [3]
pop word [bx + di - 3000]
pop sp
pop di
pop si
pop ds
xchg ax, [bp - 1000]
xchg [bx + 50], bp
xchg ax, ax
xchg ax, dx
xchg ax, sp
xchg ax, si
xchg ax, di
xchg cx, dx
xchg si, cx
xchg cl, ah
in al, 200
in al, dx
in ax, dx
out 44, ax
out dx, al
xlat
lea ax, [bx + di + 1420]
lea bx, [bp - 50]
lea sp, [bp - 1003]
lea di, [bx + si - 7]
lds ax, [bx + di + 1420]
lds bx, [bp - 50]
lds sp, [bp - 1003]
lds di, [bx + si - 7]
les ax, [bx + di + 1420]
les bx, [bp - 50]
les sp, [bp - 1003]
les di, [bx + si - 7]
lahf
sahf
pushf
popf
add cx, [bp]
add dx, [bx + si]
add [bp + di + 5000], ah
add [bx], al
add sp, 392
add si, 5
add ax, 1000
add ah, 30
add al, 9
add cx, bx
add ch, al
adc cx, [bp]
adc dx, [bx + si]
adc [bp + di + 5000], ah
adc [bx], al
adc sp, 392
adc si, 5
adc ax, 1000
adc ah, 30
adc al, 9
adc cx, bx
adc ch, al
inc ax
inc cx
inc dh
inc al
inc ah
inc sp
inc di
inc byte [bp + 1002]
inc word [bx + 39]
inc byte [bx + si + 5]
inc word [bp + di - 10044]
inc word [9349]
inc byte [bp]
aaa
daa
sub cx, [bp]
sub dx, [bx + si]
sub [bp + di + 5000], ah
sub [bx], al
sub sp, 392
sub si, 5
sub ax, 1000
sub ah, 30
sub al, 9
sub cx, bx
sub ch, al
sbb cx, [bp]
sbb dx, [bx + si]
sbb [bp + di + 5000], ah
sbb [bx], al
sbb sp, 392
sbb si, 5
sbb ax, 1000
sbb ah, 30
sbb al, 9
sbb cx, bx
sbb ch, al
dec ax
dec cx
dec dh
dec al
dec ah
dec sp
dec di
dec byte [bp + 1002]
dec word [bx + 39]
dec byte [bx + si + 5]
dec word [bp + di - 10044]
dec word [9349]
dec byte [bp]
neg ax
neg cx
neg dh
neg al
neg ah
neg sp
neg di
neg byte [bp + 1002]
neg word [bx + 39]
neg byte [bx + si + 5]
neg word [bp + di - 10044]
neg word [9349]
neg byte [bp]
