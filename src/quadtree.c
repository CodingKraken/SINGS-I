#include "main.h"

int inBoundary(Boundary boundary, Vector2 position) {
    return (position.x >= boundary.position.x-boundary.width/2 && position.x <= boundary.position.x+boundary.width/2 && position.y >= boundary.position.y-boundary.height/2 && position.y <= boundary.position.y+boundary.height/2);
}

QuadTree* initialize(Boundary boundary) {
    // Initializes a new quadtree and returns a pointer to its address in memory.
    QuadTree* temp = malloc(sizeof(QuadTree));
    temp->boundary = boundary;
    temp->particle = (Particle){
        .mass = 0,
        .position = boundary.position,
    };
    temp->ne = temp->nw = temp->sw = temp->se = NULL;
    return temp;
}

void insert(QuadTree* qt, Particle particle) {
    if(!inBoundary(qt->boundary, particle.position)) return;
    // Essentially, if no sub trees have been initialized, initialize 4 new quadtrees within the current one.
    if(!qt->ne) {
        if(!qt->particle.mass) {
            qt->particle = particle;
        } else {
            qt->ne = initialize((Boundary){
                .position.x = qt->boundary.position.x+qt->boundary.width/4, .position.y = qt->boundary.position.y+qt->boundary.height/4,
                .width = qt->boundary.width/2, .height = qt->boundary.height/2
            });
            qt->nw = initialize((Boundary){
                .position.x = qt->boundary.position.x-qt->boundary.width/4, .position.y = qt->boundary.position.y+qt->boundary.height/4,
                .width = qt->boundary.width/2, .height = qt->boundary.height/2
            });
            qt->sw = initialize((Boundary){
                .position.x = qt->boundary.position.x-qt->boundary.width/4, .position.y = qt->boundary.position.y-qt->boundary.height/4,
                .width = qt->boundary.width/2, .height = qt->boundary.height/2
            });
            qt->se = initialize((Boundary){
                .position.x = qt->boundary.position.x+qt->boundary.width/4, .position.y = qt->boundary.position.y-qt->boundary.height/4,
                .width = qt->boundary.width/2, .height = qt->boundary.height/2
            });
            // Attempt to insert the particle currently in this quadtree into the 4 subquadrants, likewise with the new particle.
            insert(qt->ne, qt->particle);
            insert(qt->nw, qt->particle);
            insert(qt->sw, qt->particle);
            insert(qt->se, qt->particle);
            insert(qt->ne, particle);
            insert(qt->nw, particle);
            insert(qt->sw, particle);
            insert(qt->se, particle);
            qt->particle.mass = qt->ne->particle.mass+qt->nw->particle.mass+qt->sw->particle.mass+qt->se->particle.mass;
            qt->particle.position = (Vector2){
                .x=(qt->ne->particle.mass*qt->ne->particle.position.x+qt->nw->particle.mass*qt->nw->particle.position.x+
                    qt->sw->particle.mass*qt->sw->particle.position.x+qt->se->particle.mass*qt->se->particle.position.x)/qt->particle.mass,
                .y=(qt->ne->particle.mass*qt->ne->particle.position.y+qt->nw->particle.mass*qt->nw->particle.position.y+
                    qt->sw->particle.mass*qt->sw->particle.position.y+qt->se->particle.mass*qt->se->particle.position.y)/qt->particle.mass};
        }
    } else {
        // If the four sub quadrants are initialized, attempt to place the particle in one of the four regions.
        insert(qt->ne, particle);
        insert(qt->nw, particle);
        insert(qt->sw, particle);
        insert(qt->se, particle);
        qt->particle.mass = qt->ne->particle.mass+qt->nw->particle.mass+qt->sw->particle.mass+qt->se->particle.mass;
        qt->particle.position = (Vector2){
            .x=(qt->ne->particle.mass*qt->ne->particle.position.x+qt->nw->particle.mass*qt->nw->particle.position.x+
                qt->sw->particle.mass*qt->sw->particle.position.x+qt->se->particle.mass*qt->se->particle.position.x)/qt->particle.mass,
            .y=(qt->ne->particle.mass*qt->ne->particle.position.y+qt->nw->particle.mass*qt->nw->particle.position.y+
                qt->sw->particle.mass*qt->sw->particle.position.y+qt->se->particle.mass*qt->se->particle.position.y)/qt->particle.mass};
    }
}

void freetree(QuadTree* qt) {
    // If there are no sub-quadtrees, free the current one and set its pointer to null
    if(!qt->ne) {
        free(qt);
        qt = NULL;
        return;
    }
    // Otherwise, attempt to free all the subtrees.
    freetree(qt->ne);
    freetree(qt->nw);
    freetree(qt->sw);
    freetree(qt->se);
}

void drawtree(QuadTree* qt, RenderTexture2D* render_texture, Vector3 camdata) {
    if(qt->particle.mass) {
        Vector2 boundary_screen = cartToScreen(qt->boundary.position, camdata, *render_texture);
        DrawRectangleLinesEx((Rectangle){
            .x = (boundary_screen.x - qt->boundary.width/2*camdata.z),
            .y = (boundary_screen.y - qt->boundary.height/2*camdata.z),
            .width = qt->boundary.width * camdata.z,
            .height = qt->boundary.height * camdata.z
        }, 1, BLACK);
    }
    if(qt->ne) {
        drawtree(qt->ne, render_texture, camdata);
        drawtree(qt->nw, render_texture, camdata);
        drawtree(qt->sw, render_texture, camdata);
        drawtree(qt->se, render_texture, camdata);
    }
}

void search(QuadTree* qt, Vector2 pos, Particle* particle) {
    if(inBoundary(qt->boundary, (Vector2){pos.x, -pos.y})) {
        if(qt->particle.mass) {
            if(!qt->ne) {
                particle->position = qt->particle.position;
            } else {
                search(qt->ne, pos, particle);
                search(qt->nw, pos, particle);
                search(qt->sw, pos, particle);
                search(qt->se, pos, particle);
            }
        }
    }
}