.macpack generic

.include "zeropage.inc"
.import incsp3
.import __hextab, pusha

.include "pixler.inc"
.importzp px_sprite_cursor

.zeropage

; TODO export as u8 and u16?
.exportzp _ix, _iy, _idx, _tmp
_ix: .word 0
_iy: .word 0
_idx: .word 0
_tmp: .word 0

.exportzp _px_sprite_cursor = px_sprite_cursor

.code

.export _meta_spr
.proc _meta_spr ; (u8 x, u8 y, u8 pal, void* meta) -> void
sprx = tmp1
spry = tmp2
pal = tmp3
meta = ptr1

	sta meta+0
	stx meta+1

	ldy #2
	lda (sp),y
	sta sprx
	
	dey
	lda (sp),y
	sta spry
	
	dey
	lda (sp),y
	sta pal
	
	ldx px_sprite_cursor

@loop:
	lda (meta), y
	cmp #$80
	beq @return
	add sprx
	sta OAM_X, x
	iny
	
	lda (meta), y
	add spry
	sta OAM_Y, x
	iny
	
	lda (meta), y
	sta OAM_CHR, x
	iny
	
	lda (meta), y
	ora pal
	sta OAM_ATTR, x
	iny
	
	inx
	inx
	inx
	inx
	jmp @loop

@return:
	stx px_sprite_cursor
	jmp incsp3
.endproc
