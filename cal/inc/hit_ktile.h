
typedef struct KHitTile {
	void	* data;
	int	origAcumCard[3];
	int 	card[3];			
} KHitTile;

#define hit_ktileNewType( type ) 	\
typedef struct KHitTile_##type {	\
	type	* data;		\
	int	origAcumCard[3];		\
	int 	card[3];			\
} KHitTile_##type;

#define hit_ktileSelect1( ref, begin0, size0 )	\
	{ ref.data+(begin0), \
		{ ref.origAcumCard[0], 0, 0 }, \
		ref.card[0], 1, 1 \
	}

#define hit_ktileSelect2( ref, begin0, size0, begin1, size1 )	\
	{ ref.data+(begin0 * ref.origAcumCard[0] + begin1), \
		{ ref.origAcumCard[0], ref.origAcumCard[1], 0 }, \
		ref.card[0], ref.card[1], 1 \
	}

#define hit_ktileSelect3( ref, begin0, size0, begin1, size1, begin2, size2 )	\
	{ ref.data+(begin0 * ref.origAcumCard[0] * ref.origAcumCard[1] + begin1 * ref.origAcumCard[1] + begin2 ), \
		{ ref.origAcumCard[0], ref.origAcumCard[1], ref.origAcumCard[2] }, \
		ref.card[0], ref.card[1], ref.card[2] \
	}


