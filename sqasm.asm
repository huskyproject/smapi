;
;   SMAPI; Modified Squish MSGAPI
;
;   Squish MSGAPI0 is copyright 1991 by Scott J. Dudley.  All rights reserved.
;   Modifications released to the public domain.
;
;   Use of this file is subject to the restrictions contain in the Squish
;   MSGAPI0 licence agreement.  Please refer to licence.txt for complete
;   details of the licencing restrictions.  If you do not find the text
;   of this agreement in licence.txt, or if you do not have this file,
;   you should contact Scott Dudley at FidoNet node 1:249/106 or Internet
;   e-mail Scott.Dudley@f106.n249.z1.fidonet.org.
;
;   In no event should you proceed to use any of the source files in this
;   archive without having accepted the terms of the MSGAPI0 licensing
;   agreement, or such other agreement as you are able to reach with the
;   author.
;

ifndef __FLAT__

.model large, pascal

else

.386p
.model small, pascal

endif

.code

public FARWRITE, FARREAD, SHARELOADED

ifndef __FLAT__

FARREAD proc
  push    bp
  mov     bp,sp
  push    ds
  mov     bx,word ptr [bp+12]  ; load file handle
  mov     cx,word ptr [bp+6]   ; load length
  mov     ax,word ptr [bp+10]  ; load segment
  mov     dx,word ptr [bp+8]   ; load offset
  mov     ds,ax                ; move the segment into DS
  mov     ah,3fh
  int     21h
  jnc     okay
  mov     ax, -1
okay:
  pop     ds
  pop     bp
  ret     8
FARREAD endp

FARWRITE proc
  push    bp
  mov     bp,sp
  push    ds
  mov     bx,[bp+12]           ; load file handle
  mov     cx,[bp+6]            ; length of write
  mov     ax,word ptr [bp+10]  ; load segment
  mov     dx,word ptr [bp+8]   ; load offset
  mov     ds,ax                ; move the segment into DS
  mov     ah,40h
  int     21h
  jnc     done_it
  mov     ax,-1
done_it:
  pop     ds
  pop     bp
  ret     8
FARWRITE endp

SHARELOADED proc
  mov     ax,1000h
  int     2fh
  cmp     al,0ffh              ; 0ffh = SHARE is loaded
  je      has_share
  xor     ax,ax
has_share:
  ret
SHARELOADED endp

else

FARREAD proc
  mov     ecx,[esp+4]          ; load length
  mov     edx,[esp+8]          ; load offset
  mov     ebx,[esp+12]         ; load file handle
  mov     ah,3fh
  int     21h
  jnc     okay
  mov     eax, -1
okay:
  ret     0ch
FARREAD endp

FARWRITE proc
  mov     ebx,[esp+12]         ; load file handle
  mov     ecx,[esp+4]          ; length of write
  mov     edx,[esp+8]          ; load offset
  mov     ah,40h
  int     21h
  jnc     done_it
  mov     eax,-1
done_it:
  ret     0ch
FARWRITE endp

SHARELOADED proc
  mov     eax,1000h
  int     2fh
  cmp     al,0ffh              ; ffh = SHARE is loaded
  je      has_share
  xor     ax,ax
has_share:
  ret
SHARELOADED endp

endif

end
