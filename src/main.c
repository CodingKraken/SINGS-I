#include "main.h"

int main(int argc, char** argv) {    

    srand(time(NULL));

    int particles, frames, read, write;
    particles = frames = read = write = 0;
    Boundary boundary = {0};
    char* file_name;
    for(int i = 1; i < argc; i++) {
        if(strstr(argv[i], "-f=")) frames = atoi(argv[i]+3);
        if(strstr(argv[i], "-p=")) particles = atoi(argv[i]+3);
        if(strstr(argv[i], "-w=")) {
            write = 1;
            file_name = strcat(strdup(argv[i]+3), ".bin");
        }
        if(strstr(argv[i], "-r=")) {
            read = 1;
            file_name = strdup(argv[i]+3);
        }
    }
    if(read) {
        if(frames || particles) {
            printf("Particle or frame specifiers invalid with read from file\n");
            return 1;
        }
    } else {
        if(!frames) {
            printf("Please specify a number of frames\n");
            return 1;
        }
        if(!particles) particles = 600;
    }
    boundary.width = 2000;
    boundary.height = 2000;

    ParticleData *particle_info;
    QuadTree* particle_tree = initialize((Boundary){.position=(Vector2){boundary.width/2,boundary.height/2},.width=10*boundary.height,.height=10*boundary.height});
    int use_bh = 1;
    if(!read) {
        printf("%i frames, %i particles to simulate\n", frames, particles);

        printf("Creating particle list\n");
        Particle* particle_list = malloc(sizeof(Particle)*particles);
        printf("Generating Galaxy\n");
        
        /*
            3 Configurations for generating particles, can be customized.
            Default is a cloud of particles with randomized initial velocities in a range
            Next is to generate a uniform distribution of particles with no mass
            And finally a double galaxy collision, where one has twice the mass of the other. NOTE: SEGFAULTS IF PARTICLES ISN'T A MULTIPLE OF 3
        */

        generateGalaxy(particle_list, particles/2, (Vector2){0,0}, (Vector2){5, 0});
        generateGalaxy(particle_list+particles/2, particles/2, (Vector2){200,0}, (Vector2){-5, 0});

        // generateParticles(particle_list, particles, particle_tree->boundary);
        
        // generateGalaxy(particle_list+particles/3, 2*particles/3, (Vector2){-boundary.width/6, 20}, (Vector2){3.f,-1.5f});
        // generateGalaxy(particle_list+2*particles/3+2, particles/3, (Vector2){-boundary.width/6, 100}, (Vector2){5.f,-0.25f});
        
        printf("%i particles simulated\n", 3*(particles/3));
        particle_info = malloc(particles * frames * sizeof(ParticleData));

        printf("Done Generating Galaxy\n");
        double elapsed = 0;

        int sec = 0;
        clock_t before = clock();

        for(int i = 0; i < frames; i++) {
            freetree(particle_tree);

            // We reinitialize the particle tree every tick then insert the particles into the tree one by one
            particle_tree = initialize((Boundary){.position=(Vector2){boundary.width/2,boundary.height/2},.width=10*boundary.height,.height=10*boundary.height});

            for(int j = 0; j < particles; j++) {
                particle_info[i*particles + j] = (ParticleData){.position=particle_list[j].position, .size=particle_list[j].mass};

                if(!inBoundary(particle_tree->boundary, particle_list[j].position)) {
                    particle_tree = initialize((Boundary){.position=particle_tree->boundary.position,.width=2*particle_tree->boundary.width,.height=2*particle_tree->boundary.height});
                    j=0;
                }
                particle_list[j].force = (Vector2){0,0};
                insert(particle_tree,particle_list[j]);
            }
            // We then call update and feeding in the simulation mode. We also increment the elapsed simulation time.
            update(particle_tree, particle_list, particles, use_bh);
            if(i == 0) {
                update(particle_tree, particle_list, particles, use_bh);
                Vector2 f1 = particle_list[0].force;
                update(particle_tree, particle_list, particles, 0);
                Vector2 f2 = particle_list[0].force;
                float err = dist(f2, f1)/(sqrtf(f2.x*f2.x + f2.y*f2.y));
                printf("Force calc err: %f", err);
            }
            if(i % 20 == 0) {
                printf("%i / %i frames done\n", i, frames);
            }
        }
        sec = (clock() - before)/CLOCKS_PER_SEC;
        printf("Computed in %i seconds\n", sec);
        free(particle_list);
        if(write) {
            if(writedata(file_name, particle_info, particles, frames))
                printf("Successfully wrote data to file %s\n", file_name);
            else
                printf("Error in writing data to file %s\n", file_name);
        }
    } else {
        if(particle_info = readdata(file_name, &particles, &frames))
            printf("Successfully read from file %s\nParticles: %d\nFrames: %d (%.2f Mya)\n", file_name, particles, frames, frames*TIMESTEP*15);
        else {
            printf("Error reading from file %s\n", file_name);
            return 1; 
        }
    }

    InitWindow(1280, 720, "SINGS I");

    RenderTexture2D screen_texture = LoadRenderTexture(1280, 720);

    SetTargetFPS(60);

    double framenum = 0;
    int incdec = 1;

    int draw_boundaries = 0;
    int paused = 1;

    Vector3 camdata = {0};
    camdata.z = 1;

    float simspeed = 1.0f;

    Particle selected_particle = {0};

    while(!WindowShouldClose()) {
        if(framenum >= frames || framenum < 0) { 
            paused = 1;
            incdec = -incdec;
            framenum = (framenum >= frames) ? frames - 1 : 0;
        }

        if(IsKeyPressed('Z')) {
            draw_boundaries = draw_boundaries == 1 ? 0 : 1;
            selected_particle.position = (Vector2){0,0};
        }
        if(IsKeyPressed('X')) selected_particle.position = (Vector2){0,0};
        if(IsKeyPressed('C')) { camdata = (Vector3){0}; camdata.z = 1; }
        if(IsKeyPressed(' ')) paused = paused == 1 ? 0 : 1;

        if(IsKeyDown('A')) camdata.x += 1;
        if(IsKeyDown('D')) camdata.x -= 1;
        if(IsKeyDown('S')) camdata.y -= 1;
        if(IsKeyDown('W')) camdata.y += 1;

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            camdata.x += GetMouseDelta().x;
            camdata.y += GetMouseDelta().y;
        }

        if(IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON) && draw_boundaries) {
            search(particle_tree, screenToCart((Vector2){GetMouseX(), GetMouseY()}, camdata, screen_texture), &selected_particle);
        }

        if(GetMouseWheelMove()) {
            camdata.z *= (GetMouseWheelMove() > 0) ? 1.1f : 1/1.1f;
            if(camdata.z <= 0) camdata.z = 0.1;
            camdata.x -= GetMouseWheelMove()*0.1;
            camdata.y -= GetMouseWheelMove()*0.1; 
            + GetMouseWheelMove()*0.1; 
        }

        if(IsKeyPressed(KEY_RIGHT)) incdec = 1;
        if(IsKeyPressed(KEY_LEFT)) incdec = -1;
        if(IsKeyDown(KEY_UP)) simspeed *=1.01f;
        if(IsKeyDown(KEY_DOWN)) simspeed /= 1.01f;
        if(IsKeyPressed('X')) simspeed = 1.f;

        if(simspeed <= 0.01f) simspeed = 0.01f;

        BeginTextureMode(screen_texture);
            ClearBackground(WHITE);

            for(int j = 0; j < particles; j++) {
                ParticleData current = particle_info[((int)framenum) * particles + j];
                Vector2 screencords = cartToScreen(current.position, camdata, screen_texture);
                DrawCircle(screencords.x, screencords.y, 1 + (current.size/1000), BLACK);
            }

            Vector2 screen_start = cartToScreen((Vector2){-720/2, 0}, camdata, screen_texture);
            Vector2 screen_end = cartToScreen((Vector2){1280/2, 0}, camdata, screen_texture);

            if(draw_boundaries) {
                freetree(particle_tree);
                particle_tree = initialize((Boundary){.position=(Vector2){boundary.width/2,boundary.height/2},.width=10*boundary.height,.height=10*boundary.height});
                for(int j = 0; j < particles; j++) {
                    if(!inBoundary(particle_tree->boundary, particle_info[((int)framenum)*particles + j].position)) {
                        particle_tree = initialize((Boundary){.position=particle_tree->boundary.position,.width=2*particle_tree->boundary.width,.height=2*particle_tree->boundary.height});
                        j=0;
                    }
                    insert(particle_tree,(Particle){.mass=particle_info[((int)framenum)*particles+j].size, .position=particle_info[((int)framenum)*particles + j].position});
                }
                if(!selected_particle.position.x)
                    drawtree(particle_tree, &screen_texture, camdata);
                else
                    bhTreeDraw(particle_tree, selected_particle, &screen_texture, camdata);
            }

        EndTextureMode();

        if(!paused)
            framenum += incdec*simspeed;

        // Drawing the scene!
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureRec(screen_texture.texture, (Rectangle){0,0, 1280, 720},(Vector2){0,0}, WHITE);

            // All text objects are simply rendered one by one, without respect to camera position.
            DrawText(TextFormat("t=%.2f Mya", framenum*TIMESTEP*15), 20, 20, 20, WHITE);
            DrawText(TextFormat("x=%.1f y=%.1f", screenToCart(GetMousePosition(), camdata, screen_texture).x, screenToCart(GetMousePosition(), camdata, screen_texture).y), 200, 20, 20, WHITE);
            DrawText(TextFormat("x=%.1f y=%.1f", camdata.x, camdata.y), 200, 40, 20, WHITE);
            DrawText(TextFormat("s=%.2f", simspeed), 20, 40, 20, WHITE);
            DrawText(TextFormat("bh=%d", use_bh), 20, 60, 20, WHITE);
            DrawFPS(20,80);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}