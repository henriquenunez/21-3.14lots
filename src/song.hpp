#include "beep.hpp"
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "shader.h"

struct Note
{
    double freq; // Pitch
    int duration;
};

double freq_mut()
{
    return static_cast<double>(-200 + rand() % 401) * 0.2;
}

int dur_mut()
{
    return (-400 + rand() % 801) * 0.2;
}

class Song
{
public:
    Song()
    {
        _triangle_count = 0;
    }

    static Song randInit(int n)
    {
        Song gen;
        
        for (int i = 0 ; i < n ; i++)
        {
            gen.notes.push_back({static_cast<double>(rand() % 1780), 100 + rand() % 800});
        }
        
        gen.initGL();

        return gen;
    }
        
    static Song genFromParents(Song &a, Song &b)
    {
        Song gen;
        
        for (int i = 0 ; i < a.notes.size() ; i++)
        {
            gen.notes.push_back({freq_mut() + (a.notes[i].freq + b.notes[i].freq)/2.0, 
        			 dur_mut() + (a.notes[i].duration + b.notes[i].duration)/2});
        }
        
        gen.initGL();

        return gen;
    }

    std::vector<Note> notes;
    int score;

    void play()
    {
        Beeper b;
	    for (const Note &a : notes)
	    {
            b.beep(a.freq, a.duration);
	    }
        b.wait();
    }

    void render()
    {
        glBindVertexArray(_vao);

        //IMPORTANT: the third parameter is the number of vertices!
        glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
    }

private:
    unsigned int _vao, _vbo;
    size_t _triangle_count;
    std::vector<float> vertices;

    #define NOTE_HEIGHT_GUI 0.02f

    // Remember that the vertex attributes are:
    // x, y
    // just those for now.
    void initGL()
    {
        get_ui_vertices_song();

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        glGenBuffers(1, &_vbo);

        std::cout << "Generated " << vertices.size() << " vertices\n";

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER,
                vertices.size() * sizeof(float),
                &vertices[0],
                GL_STATIC_DRAW);

                                                        // Stride is = 2
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
        // glEnableVertexAttribArray(1);
    }

    void get_ui_vertices_song()
    {
        // Creating vertices
        vertices.clear();
        float last_x = 0.0, t_x, t_y, dy;

        for (Note &a_note : notes)
        {
            const float time_offset = (float) a_note.duration / 10000.0f;

            dy = a_note.freq / 1000.0; //440.0;
         
            // Upper triangle
            // First vertex
            t_x = last_x;
            t_y = dy;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            // Second vertex
            t_x = last_x;
            t_y = dy + NOTE_HEIGHT_GUI;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            // Third vertex
            t_x = last_x + time_offset;
            t_y = dy + NOTE_HEIGHT_GUI;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            _triangle_count += 1;

            // Lower triangle
            // First vertex
            t_x = last_x;
            t_y = dy;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            // Second vertex
            t_x = last_x + time_offset;
            t_y = dy + NOTE_HEIGHT_GUI;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            // Third vertex
            t_x = last_x + time_offset;
            t_y = dy;

            vertices.push_back(t_x);
            vertices.push_back(t_y);

            _triangle_count += 1;

            last_x += time_offset;
        }
    }

};

class Population
{
public:
    Population(int n)
    {
    	for (int i = 0 ; i < n ; i++)
    	{
    	    songs.push_back(Song::randInit(10));
    	}
    }

    void playPopulation()
    {
    	int cnt = 1;
    	for (Song &a : songs)
    	{
    	    #ifdef DEBUG
    	    printf("Playing song nº %d\n", cnt++);
    	    #endif

    	    a.play();
    	}
    }

    std::vector<Song> songs;
    int iter_n;
    int best_score = 0, best_idx = 0;
    int last_played = -1;

    void evaluate() // This is our most complicated function.
    {
    	int cnt = 0;
    	for (Song &a : songs)
    	{
            last_played = cnt;
    	    //a.play();

    	    int score = cnt;
    	    // printf("Grade for song nº %d\n", cnt);
    	    scanf("%d", &score);

    	    if (score > best_score)
    	    {
        		best_score = score;
        		best_idx = cnt;
    	    }
	        a.score = score;
	        cnt++;
    	}
    }

    void draw_last_played()
    {
        if (last_played >= 0)
            songs[last_played].render();
    }

    float fitness(Song &a)
    {
    }

    void elitism()
    {
	   for (int i = 0 ; i < songs.size() ; i++)
	   {
	       if (i == best_idx) continue;
	       songs[i] = Song::genFromParents(songs[best_idx], songs[i]);
	   }
    }
};

