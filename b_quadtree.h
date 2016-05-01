/**

Blake Trahan
https://github.com/blaketrahan/b_libs/

QuadTree:
Keeps blist of objects in quadtree.
Uses transient memory for splitting quads and keeping new blists.
Transient memory should be emptied at the end of every physics step

*/

const s4 MAX_ENTITIES = 10;
const s4 MAX_LEVELS = 4;
const f4 QUAD_SIZE = 4000.0f;

struct QuadTree
{ 
	struct Quad { 
		blist list_enemies;
		s4 level;
		b4 has_children;
		f4 width;
		vec2 pos;
		Quad* quads;
	};

	Quad* quads;
};

void split_quad(QuadTree* tree, QuadTree::Quad* quad);

void init_quads(QuadTree* tree)
{
	tree->quads = (QuadTree::Quad*)alloc(memory,sizeof(QuadTree::Quad) * 4);

	for (s4 i = 0; i < 4; i++) {
		tree->quads[i].list_enemies.set(memory,sizeof(Enemy*),MAX_ENTITIES * 2);
		tree->quads[i].has_children = false;
		tree->quads[i].level = 1;
		tree->quads[i].quads = 0;
		tree->quads[i].width = QUAD_SIZE;
	}
	tree->quads[0].pos = vec2(-QUAD_SIZE/2.0f,QUAD_SIZE/2.0f);
	tree->quads[1].pos = vec2(QUAD_SIZE/2.0f,QUAD_SIZE/2.0f);
	tree->quads[2].pos = vec2(QUAD_SIZE/2.0f,-QUAD_SIZE/2.0f);
	tree->quads[3].pos = vec2(-QUAD_SIZE/2.0f,-QUAD_SIZE/2.0f);
} 

void clear_quads(QuadTree* tree)
{
	for (s4 i = 0; i < 4; i++) {
		tree->quads[i].list_enemies.length = 0;
		tree->quads[i].has_children = false;   
	}
}

void add_to_quad(QuadTree* tree, QuadTree::Quad* quad, void* obj)
{
	quad->list_enemies.push(obj);
	if (quad->list_enemies.size() > MAX_ENTITIES && quad->level < MAX_LEVELS)
	{
		split_quad(tree,quad);
	}
}

void find_and_add_to_quad(QuadTree* tree, void* obj, vec2 pos, QuadTree::Quad* found_quad = 0, vec2 quad_pos = vec2(0,0))
{  
	QuadTree::Quad* quads;

	if (found_quad == 0) {
		quads = tree->quads;
	} else {
		quads = found_quad->quads;
	}
	 
	if (pos.x <= quad_pos.x && pos.y >= quad_pos.y)
		found_quad = &quads[0];
	else if (pos.x >= quad_pos.x && pos.y >= quad_pos.y)
		found_quad = &quads[1];
	else if (pos.x >= quad_pos.x && pos.y <= quad_pos.y)
		found_quad = &quads[2];
	else if (pos.x <= quad_pos.x && pos.y <= quad_pos.y)
		found_quad = &quads[3];

	if (found_quad->has_children)
	{
		find_and_add_to_quad(tree,obj,pos,found_quad, found_quad->pos);
	}
	else
	{ 
		add_to_quad(tree, found_quad, obj);
	}
}

void split_quad(QuadTree* tree, QuadTree::Quad* quad)
{
	quad->has_children = true;
	quad->quads = (QuadTree::Quad*)alloc_transient(memory,sizeof(QuadTree::Quad) * 4);
	s4 level = quad->level+1;

	f4 half_width = QUAD_SIZE;
	for (s4 i = 0; i < level; i++)
	{
		half_width /= 2.0f;
	}

	for (s4 i = 0; i < 4; i++)
	{
		quad->quads[i].list_enemies.set(memory,sizeof(Enemy*),MAX_ENTITIES * 2, true);
		quad->quads[i].has_children = false; 
		quad->quads[i].level = level;
		quad->quads[i].width = half_width * 2.0f;
		quad->quads[i].quads = 0;
	}
	
	quad->quads[0].pos = quad->pos + vec2(-half_width,half_width);
	quad->quads[1].pos = quad->pos + vec2(half_width,half_width);
	quad->quads[2].pos = quad->pos + vec2(half_width,-half_width);
	quad->quads[3].pos = quad->pos + vec2(-half_width,-half_width); 

	blist* ent_list = &quad->list_enemies;
	for (s4 i = 0; i < ent_list->size(); i++)
	{
		Enemy* enemy = (Enemy*)(*ent_list)[i];
		find_and_add_to_quad(tree, enemy, enemy->curr_pos, quad, quad->pos);
	}
}

QuadTree::Quad* get_quad_from_pos(QuadTree* tree, vec2 pos, QuadTree::Quad* found_quad = 0, vec2 quad_pos = vec2(0,0))
{
	QuadTree::Quad* quads;

	if (found_quad == 0) {
		quads = tree->quads;
	} else {
		quads = found_quad->quads;
	}
	 
	if (pos.x <= quad_pos.x && pos.y >= quad_pos.y)
		found_quad = &quads[0];
	else if (pos.x >= quad_pos.x && pos.y >= quad_pos.y)
		found_quad = &quads[1];
	else if (pos.x >= quad_pos.x && pos.y <= quad_pos.y)
		found_quad = &quads[2];
	else if (pos.x <= quad_pos.x && pos.y <= quad_pos.y)
		found_quad = &quads[3];

	if (found_quad->has_children)
	{
		get_quad_from_pos(tree,pos,found_quad, found_quad->pos);
	}
	else
	{
		return found_quad;
	}
}

blist* get_list_from_quad(QuadTree* tree, vec2 pos)
{
	QuadTree::Quad* fq = get_quad_from_pos(tree,pos);
	return &fq->list_enemies;
}