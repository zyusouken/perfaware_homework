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
cbw
cwd