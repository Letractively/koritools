/ /   f i l e t r e e 2 x m l . c p p   :   D e f i n e s   t h e   e n t r y   p o i n t   f o r   t h e   c o n s o l e   a p p l i c a t i o n .  
 / /  
  
 # i n c l u d e   " s t d a f x . h "  
 # i n c l u d e   < w i n d o w s . h >  
  
 / /   x e r c e s   i n c l u d e s  
 # i n c l u d e   < x e r c e s c / u t i l / P l a t f o r m U t i l s . h p p >  
 # i n c l u d e   < x e r c e s c / u t i l / X M L S t r i n g . h p p >  
 # i n c l u d e   < x e r c e s c / d o m / D O M . h p p >  
 # i n c l u d e   < x e r c e s c / u t i l / O u t O f M e m o r y E x c e p t i o n . h p p >  
 # i n c l u d e   < x e r c e s c / f r a m e w o r k / S t d O u t F o r m a t T a r g e t . h p p >  
  
  
 # i n c l u d e   < i o s t r e a m >  
  
 X E R C E S _ C P P _ N A M E S P A C E _ U S E  
  
 	  
 v o i d   S a v e F i l e T r e e T o X M L ( c o n s t   X M L C h *   R o o t P a t h ,   D O M N o d e *   R o o t N o d e ,   X E R C E S _ C P P _ N A M E S P A C E : : D O M D o c u m e n t *   D o c u m e n t )  
 {  
 	 W I N 3 2 _ F I N D _ D A T A   F i n d D a t a ;  
 	 Z e r o M e m o r y (   & F i n d D a t a ,   s i z e o f ( W I N 3 2 _ F I N D _ D A T A ) ) ;  
 	 X M L S i z e _ t   S e a r c h P a t t e r n L e n   =   X M L S t r i n g : : s t r i n g L e n (   R o o t P a t h   )   +   1   +     1 ;  
 	 X M L C h *   S e a r c h P a t t e r n   =   n e w   X M L C h [ S e a r c h P a t t e r n L e n ] ;  
 	 Z e r o M e m o r y ( S e a r c h P a t t e r n ,   s i z e o f ( X M L C h ) * S e a r c h P a t t e r n L e n   ) ;  
 	 X M L S t r i n g : : c a t S t r i n g (   S e a r c h P a t t e r n ,   R o o t P a t h   ) ;  
 	 X M L S t r i n g : : c a t S t r i n g (   S e a r c h P a t t e r n ,   L " * " ) ;  
 	  
 	 H A N D L E   h S e a r c h   =   F i n d F i r s t F i l e ( S e a r c h P a t t e r n ,   & F i n d D a t a ) ;  
 	 d e l e t e [ ]   S e a r c h P a t t e r n ;  
 	 i f (   I N V A L I D _ H A N D L E _ V A L U E   = =   h S e a r c h   )  
 	 	 r e t u r n ;  
 	 d o  
 	 {  
 	 	 X M L C h *   F i l e N a m e   =   F i n d D a t a . c F i l e N a m e ;  
 	 	 i f (   F i n d D a t a . d w F i l e A t t r i b u t e s   &   F I L E _ A T T R I B U T E _ D I R E C T O R Y   )  
 	 	 {  
 	 	 	 i f (   ( X M L S t r i n g : : c o m p a r e I S t r i n g ( F i l e N a m e , L " . " ) ! = 0 )   & &  
 	 	 	 	 ( X M L S t r i n g : : c o m p a r e I S t r i n g ( F i l e N a m e , L " . . " ) ! = 0 )   )  
 	 	 	 {  
 	 	 	 	 D O M E l e m e n t *   d i r   =   D o c u m e n t - > c r e a t e E l e m e n t ( L " d i r e c t o r y " ) ;  
 	 	 	 	 d i r - > s e t A t t r i b u t e ( L " n a m e " ,   F i l e N a m e   ) ;  
 	 	 	 	 R o o t N o d e - > a p p e n d C h i l d ( d i r ) ;  
 	 	 	 	 X M L S i z e _ t   S u b D i r P a t h L e n   =   X M L S t r i n g : : s t r i n g L e n (   R o o t P a t h   )   +   X M L S t r i n g : : s t r i n g L e n (   F i l e N a m e   )   +   1     +   1 ;  
 	 	 	 	 X M L C h *   S u b D i r   =   n e w   X M L C h [   S u b D i r P a t h L e n   ] ;  
 	 	 	 	 Z e r o M e m o r y (   S u b D i r ,   s i z e o f ( X M L C h ) * S u b D i r P a t h L e n   ) ;  
 	 	 	 	 X M L S t r i n g : : c a t S t r i n g (   S u b D i r ,   R o o t P a t h   ) ;  
 	 	 	 	 X M L S t r i n g : : c a t S t r i n g (   S u b D i r ,   F i l e N a m e ) ;  
 	 	 	 	 X M L S t r i n g : : c a t S t r i n g (   S u b D i r ,   L " \ \ "   ) ;  
 	 	 	 	 S a v e F i l e T r e e T o X M L (   S u b D i r ,   d i r ,   D o c u m e n t   ) ;  
 	 	 	 	 d e l e t e [ ]   S u b D i r ;  
 	 	 	 }  
 	 	 }  
 	 	 e l s e  
 	 	 {  
 	 	 	 D O M E l e m e n t *   f i l e   =   D o c u m e n t - > c r e a t e E l e m e n t ( L " f i l e " ) ;  
 	 	 	 f i l e - > s e t A t t r i b u t e ( L " n a m e " ,   F i l e N a m e   ) ;  
 	 	 	 R o o t N o d e - > a p p e n d C h i l d ( f i l e ) ;  
 	 	 }  
 	 } w h i l e (   F i n d N e x t F i l e (   h S e a r c h ,   & F i n d D a t a )   ! =   0   ) ;  
 	 F i n d C l o s e (   h S e a r c h   ) ;  
 }  
  
  
 v o i d   S a v e X M L T o F i l e ( c o n s t   X M L C h *   F i l e N a m e ,   X E R C E S _ C P P _ N A M E S P A C E : : D O M D o c u m e n t *   D o c u m e n t   )  
 {  
  
 }  
  
 v o i d   P r i n t X M L d o c ( c o n s t   X E R C E S _ C P P _ N A M E S P A C E : : D O M D o c u m e n t *   d o c )  
 {  
 	 D O M I m p l e m e n t a t i o n *   i m p l   =     D O M I m p l e m e n t a t i o n R e g i s t r y : : g e t D O M I m p l e m e n t a t i o n ( L " C o r e " ) ;  
 	 D O M L S S e r i a l i z e r       * t h e S e r i a l i z e r   =   ( ( D O M I m p l e m e n t a t i o n L S * ) i m p l ) - > c r e a t e L S S e r i a l i z e r ( ) ;  
         D O M L S O u t p u t               * t h e O u t p u t D e s c   =   ( ( D O M I m p l e m e n t a t i o n L S * ) i m p l ) - > c r e a t e L S O u t p u t ( ) ;  
 	  
 	 D O M C o n f i g u r a t i o n *   s e r i a l i z e r C o n f i g = t h e S e r i a l i z e r - > g e t D o m C o n f i g ( ) ;  
 	 i f   ( s e r i a l i z e r C o n f i g - > c a n S e t P a r a m e t e r ( X M L U n i : : f g D O M W R T F o r m a t P r e t t y P r i n t ,   t r u e ) )  
 	 	 s e r i a l i z e r C o n f i g - > s e t P a r a m e t e r ( X M L U n i : : f g D O M W R T F o r m a t P r e t t y P r i n t ,   t r u e ) ;  
  
 	 X M L F o r m a t T a r g e t   * o u t p u t S t r e a m   =   n e w   S t d O u t F o r m a t T a r g e t ( ) ;  
 	 t h e O u t p u t D e s c - > s e t B y t e S t r e a m ( o u t p u t S t r e a m ) ;  
  
 	 t h e S e r i a l i z e r - > w r i t e ( d o c ,   t h e O u t p u t D e s c ) ;  
  
         t h e O u t p u t D e s c - > r e l e a s e ( ) ;  
         t h e S e r i a l i z e r - > r e l e a s e ( ) ;  
  
 	 d e l e t e   o u t p u t S t r e a m ;  
 }  
  
 i n t   _ t m a i n ( i n t   a r g c ,   _ T C H A R *   a r g v [ ] )  
 {  
 	 t r y  
 	 {  
 	 	 X M L P l a t f o r m U t i l s : : I n i t i a l i z e ( ) ;  
 	 }  
 	 c a t c h ( . . . )  
 	 {  
 	 	 r e t u r n   1 ;  
 	 }  
  
 	 X M L C h *   P a t h     =   N U L L ;  
 	 D O M I m p l e m e n t a t i o n *   i m p l   =     D O M I m p l e m e n t a t i o n R e g i s t r y : : g e t D O M I m p l e m e n t a t i o n ( L " C o r e " ) ;  
 	 X E R C E S _ C P P _ N A M E S P A C E : : D O M D o c u m e n t *   d o c   =   i m p l - > c r e a t e D o c u m e n t (   0 ,   L " d i r e c t o r y " ,   0 ) ;  
 	  
 	 i f (   a r g c   > =   2 )  
 	 	 P a t h   =   a r g v [ 1 ] ;  
 	 e l s e  
 	 	 P a t h   =   L " . \ \ " ;  
 	 S a v e F i l e T r e e T o X M L ( P a t h ,   d o c - > g e t D o c u m e n t E l e m e n t ( )   , d o c ) ;  
  
 	 P r i n t X M L d o c (   d o c   ) ;  
  
 	 d o c - > r e l e a s e ( ) ;  
  
 	 X M L P l a t f o r m U t i l s : : T e r m i n a t e ( ) ;  
 	 r e t u r n   0 ;  
 }  
  
 