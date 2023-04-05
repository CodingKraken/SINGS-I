#include "main.h"

// Generates particles randomly somewhere in the screen, with a random mass
void generateParticles(Particle* particle_list, int num_particles, Boundary boundary) {
    for(int i = 0; i < num_particles; i++) {
        particle_list[i].mass =  randrangeBiased(100, 1000);
        particle_list[i].position.x = randrange(-boundary.width/2, boundary.width/2);
        particle_list[i].position.y = randrange(-boundary.height/2, boundary.height/2);
        particle_list[i].velocity = (Vector2){0, 0};
        particle_list[i].force = (Vector2){0, 0};
    }
}

void generateGalaxy(Particle* particle_list, int num_particles, Vector2 pos,  Vector2 pec) {
    for(int i = 0; i < num_particles; i++) {
        if(i == 0) {
            // At the center of each galaxy, generate a particle that has the mass twice that of all other particles. This bounds all the particles to the galaxy.
            particle_list[i].mass = 2*num_particles;
            particle_list[i].position = pos; 
            particle_list[i].velocity = pec;
            particle_list[i].force = (Vector2){0,0};
            continue;
        }
        // Generate a random radius and angle from the center of the galaxy, then give it a required velocity to have an initially circular orbit about the center.
        double randr = randrangeBiased(1, 50);
        double randtheta = randrange(0, 2*PI);
        Vector2 randpos = (Vector2){randr*cosf(randtheta)+pos.x, randr*sinf(randtheta)+pos.y};

        particle_list[i].mass = 1;
        particle_list[i].position = randpos;
        particle_list[i].velocity = (Vector2){pec.x,pec.y};
        particle_list[i].force = (Vector2){0,0};
    }
    for(int j = 0; j < num_particles; j++) {
        newtonGravCalc(particle_list, num_particles, &particle_list[j]);
        double r = dist(particle_list[j].position, pos);
        particle_list[j].velocity.x += -sgn(particle_list[j].force.y)*sqrtl(r*absl(particle_list[j].force.y)/particle_list[j].mass); 
        particle_list[j].velocity.y += sgn(particle_list[j].force.x)*sqrtl(r*absl(particle_list[j].force.x)/particle_list[j].mass/2);
        printf("Done generating particle %i\n", j);
    }
}

void generateCloud(Particle* particle_list, int num_particles, Vector2 pos, Vector2 max_mag) {
    double randr = randrangeBiased(1, 30);
    double randtheta = randrange(0, 2*PI);
    Vector2 randpos = (Vector2){randr*cosf(randtheta)+pos.x, randr*sinf(randtheta)+pos.y};

    for(int i = 0; i < num_particles; i++) {
        particle_list[i].mass = 1;
        particle_list[i].position = randpos;
        particle_list[i].velocity = (Vector2){randrange(-max_mag.x,max_mag.x), randrange(-max_mag.y, max_mag.y)};
        particle_list[i].force = (Vector2){0,0};
    }
}

void gravIntegrateNewton(Particle* particle_list, int num_particles) {
    for(int i = 0; i < num_particles; i++) {
        newtonGravCalc(particle_list, num_particles, &particle_list[i]);
        particle_list[i].velocity.x += particle_list[i].force.x/particle_list[i].mass * TIMESTEP;
        particle_list[i].velocity.y += particle_list[i].force.y/particle_list[i].mass * TIMESTEP;
        particle_list[i].position.x += particle_list[i].velocity.x * TIMESTEP;
        particle_list[i].position.y += particle_list[i].velocity.y * TIMESTEP;
    }
}

void newtonGravCalc(Particle* particle_list, int num_particles, Particle* particle) {
    double force_mag;
    for(int i = 0; i < num_particles; i++) {
        double o_dist = dist(particle->position, particle_list[i].position);
        if(o_dist != 0) {
            Vector2 normal = normalize(particle->position, particle_list[i].position);

            // Calculate the force due to gravity between the two particles, then add that to the particle's force
            force_mag = o_dist * G * particle->mass * particle_list[i].mass/pow(o_dist*o_dist + E*E,1.5f);
            particle->force.x += force_mag*normal.x;
            particle->force.y += force_mag*normal.y;
        }
    }
}

void gravIntegrateBH(QuadTree* qt, Particle* particle_list, int num_particles) {
    for(int i = 0; i < num_particles; i++) {
        // Calculate the sum of the forces on the particle via Barnes-Hut
        bhGravCalc(qt, &particle_list[i]);

        // Update particle velocities and positions after calculating forces
        particle_list[i].velocity.x += particle_list[i].force.x/particle_list[i].mass*TIMESTEP;
        particle_list[i].velocity.y += particle_list[i].force.y/particle_list[i].mass*TIMESTEP;
        particle_list[i].position.x += particle_list[i].velocity.x*TIMESTEP;
        particle_list[i].position.y += particle_list[i].velocity.y*TIMESTEP;
    }
}

void bhGravCalc(QuadTree* qt, Particle* particle) {
    double force;
    double o_dist;
    // If this node only contains one particle, do the force calculation
    if(!qt->ne && dist(qt->particle.position, particle->position) != 0 && qt->particle.mass) {
        o_dist = dist(particle->position, qt->particle.position);
        force = o_dist * G * particle->mass * qt->particle.mass/pow(o_dist*o_dist + E*E,1.5f);
        particle->force.x += force*normalize(particle->position, qt->particle.position).x;
        particle->force.y += force*normalize(particle->position, qt->particle.position).y;
    } else if(qt->boundary.width/dist(qt->particle.position, particle->position) < BH_THRESHOLD) {
        o_dist = dist(particle->position, qt->particle.position);
        force = o_dist * G * particle->mass * qt->particle.mass/pow(o_dist*o_dist + E*E,1.5f);
        particle->force.x += force*normalize(particle->position, qt->particle.position).x;
        particle->force.y += force*normalize(particle->position, qt->particle.position).y;
    } else if(qt->ne) {
        // Otherwise keep going down the tree
        bhGravCalc(qt->ne, particle);
        bhGravCalc(qt->nw, particle);
        bhGravCalc(qt->sw, particle);
        bhGravCalc(qt->se, particle);
    }
}

void update(QuadTree* qt, Particle* particle_list, int num_particles, int BH) {
    // If we're using Barnes-Hut, use that integrator, if not, use the naive approach
    if(BH) {
        gravIntegrateBH(qt, particle_list, num_particles);
     } else { 
        gravIntegrateNewton(particle_list, num_particles);
     }
}

void bhTreeDraw(QuadTree* qt, Particle particle, RenderTexture2D* render_texture, Vector3 camdata) {
        // If this node only contains one particle, draw the tree
    if(!qt->ne && qt->particle.mass) {
        Vector2 boundary_screen = cartToScreen(qt->boundary.position, camdata, *render_texture);
        Color color = BLACK;
        if(dist(qt->particle.position, particle.position) == 0) color = RED;
        DrawRectangleLinesEx((Rectangle){
            .x = (boundary_screen.x - qt->boundary.width/2*camdata.z),
            .y = (boundary_screen.y - qt->boundary.height/2*camdata.z),
            .width = qt->boundary.width*camdata.z,
            .height = qt->boundary.height*camdata.z
        }, 1, color);
    } else if(qt->boundary.width/dist(qt->particle.position, particle.position) < BH_THRESHOLD && qt->particle.mass) {
        Vector2 boundary_screen = cartToScreen(qt->boundary.position, camdata, *render_texture);
        // If not, draw the boundary of the larger tree if it meets the barnes-hut criteria
        DrawRectangleLinesEx((Rectangle){
            .x = (boundary_screen.x - qt->boundary.width/2*camdata.z),
            .y = (boundary_screen.y - qt->boundary.height/2*camdata.z),
            .width = qt->boundary.width*camdata.z,
            .height = qt->boundary.height*camdata.z
        }, 1, BLACK);
    } else if(qt->ne) {
        // Otherwise keep going down the tree
        bhTreeDraw(qt->ne, particle, render_texture, camdata);
        bhTreeDraw(qt->nw, particle, render_texture, camdata);
        bhTreeDraw(qt->sw, particle, render_texture, camdata);
        bhTreeDraw(qt->se, particle, render_texture, camdata);
    }
}