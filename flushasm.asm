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

; Truly flush a file handle to disk

ifndef __FLAT__

.model large, pascal

else

.386p
.model small, pascal

endif

.code

public FLUSH_HANDLE2

ifndef __FLAT__

; duplicate file handle, then close the duplicate.

FLUSH_HANDLE2 proc
  push    bp
  mov     bp,sp
  mov     ah,45h
  mov     bx,[bp+6]
  int     21h
  jc      err
  mov     bx,ax
  mov     ah,3eh
  int     21h
err:
  pop     bp
  ret     2
FLUSH_HANDLE2 endp

else

FLUSH_HANDLE2 proc
  mov     ah,45h
  mov     ebx,[esp+4]
  int     21h
  jc      err
  mov     ebx,eax
  mov     ah,3eh
  int     21h
err:
  ret     4
FLUSH_HANDLE2 endp

endif

end
