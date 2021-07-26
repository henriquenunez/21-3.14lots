#ifndef SONG_H
#define SONG_H

#include "beep.hpp"
#include <SDL/SDL_timer.h>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <memory>
#include <iostream>
#include <mutex>
#include "shader.h"

#define KEYS 88 // I like pianos

struct Note
{
    //double freq; // Pitch
    int note_n; // If 0 it's a pause.
    int duration;

    static inline double get_n_freq(int n)
    {
        return pow(2, (float)(n - 49)/12.0) * 440.0;
    }

    bool is_pause()
    {
	if (note_n == 0) return true;
	return false;
    }

    inline double get_freq()
    {
        return get_n_freq(note_n);
    }
};

double freq_mut()
{
    return static_cast<double>(-200 + rand() % 401) * 0.2;
}

int note_mut()
{
    return -2 + rand() % 3;
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
        is_gl_ok = false;
    }

    ~Song()
    {
        std::cout << "Song destructor\n";
	cleanGraphics();
    }

    static Song read_song(const char* filename)
    {
	Song gen;
        
        //for (int i = 0 ; i < n ; i++)
        //{
        //    gen.notes.push_back({(rand() % KEYS), 100 + rand() % 800});
        //}
        
        FILE* song_f = fopen(filename, "r");
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
	int temp_note, temp_duration;

        if (song_f == NULL)
            exit(EXIT_FAILURE);
    
        while ((read = getline(&line, &len, song_f)) != -1)
        {
    	// Checking if line is comment
            if (line[0] == '#') continue;
	    sscanf(line, "%d %d", &temp_note, &temp_duration); 
            printf("Retrieved line of length %zu:\n", read);
            printf("%s", line);
	    gen.notes.push_back({temp_note, temp_duration * 250});
        }
    
        fclose(song_f);
        if (line)
            free(line);
        return gen;
        //exit(EXIT_SUCCESS);
    }
    //void operator = (const Song &s)
    //{
    //    cleanGraphics();
    //    notes = s.notes;
    //}

    /* 
    Song(int n)
    {
        _triangle_count = 0;
        is_gl_ok = false;

        for (int i = 0 ; i < n ; i++)
        {
            notes.push_back({(rand() % KEYS), 100 + rand() % 800});
        }
    }

    Song (Song &a, Song &b)
    {
        _triangle_count = 0;
        is_gl_ok = false;

        for (int i = 0 ; i < a.notes.size() ; i++)
        {
            notes.push_back({note_mut() + (a.notes[i].note_n + b.notes[i].note_n)/2, 
        			             dur_mut() + (a.notes[i].duration + b.notes[i].duration)/2});
        }
        
    }
    */


    static Song randInit(int n)
    {
        Song gen;
        
        for (int i = 0 ; i < n ; i++)
        {
            gen.notes.push_back({(rand() % KEYS), 100 + rand() % 800});
        }
        
        //gen.initGL();

        return gen;
    }
        
    static Song genFromParents(Song &a, Song &b)
    {
        Song gen;
        
        for (int i = 0 ; i < a.notes.size() ; i++)
        {
            gen.notes.push_back({note_mut() + (a.notes[i].note_n + b.notes[i].note_n)/2, 
        			             dur_mut() + (a.notes[i].duration + b.notes[i].duration)/2});
        }
        
        //gen.initGL();

        return gen;
    }
    
    std::vector<Note> notes;
    int score;
    int played_note = -1;

    void play()
    {
        played_note = 0;
        Beeper b;
	    for (Note &a : notes)
	    {
		std::cout << "Played note is: " << played_note << "\n";
		if (a.is_pause()) SDL_Delay(a.duration);
		else b.beep(a.get_freq(), a.duration); // Speeding up a bit 
            	b.wait();
            	played_note++;
	    }
        played_note = -1;
    }

    void render(shader_t *shader)
    {
        glBindVertexArray(_vao);

        if (played_note == -1)
        {
            glDrawArrays(GL_TRIANGLES, 0, _triangle_count * 3);
            return;            
        }

        // Splitting in 3 draw calls to color the notes
        glDrawArrays(GL_TRIANGLES, 0, played_note * 2 * 3); // White before
        set_uniform_float3(shader, 1.0, 0.0, 0.5, "color");
        glDrawArrays(GL_TRIANGLES, played_note * 2 * 3, 2 * 3); // Red current
        set_uniform_float3(shader, 1.0, 1.0, 1.0, "color");
        glDrawArrays(GL_TRIANGLES, (played_note + 1) * 2 * 3, (notes.size() - played_note - 1) * 2 * 3); // White after

	glBindVertexArray(0);
    }

    void cleanGraphics()
    {
        if (is_gl_ok)
        {
	    std::cout << "Cleaning graphics\n";
	    glDeleteBuffers(1, &_vbo);
            glDeleteVertexArrays(1, &_vao);
        }
    }

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
        std::cout << "vbo " << _vbo << " vao " << _vao << "\n";

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER,
                vertices.size() * sizeof(float),
                &vertices[0],
                GL_STATIC_DRAW);

                                                        // Stride is = 2
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

	glBindVertexArray(0);
        
	is_gl_ok = true;
    }

    bool is_gl_ok;
private:
    unsigned int _vao, _vbo;
    size_t _triangle_count;
    std::vector<float> vertices;

    float step_note = 2.0 / (float)KEYS;

    void get_ui_vertices_song()
    {
        // Creating vertices
        vertices.resize(0);
	_triangle_count = 0;
        float last_x = 0.0, t_x, t_y, dy;

        for (Note &a_note : notes)
        {
            const float time_offset = (float) a_note.duration / 10000.0f;

            dy = -1.0 + a_note.note_n * step_note;
         
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

            last_x += time_offset + 0.005;
        }
    }

};

class Population
{
public:
    Population(int n)
    {
        songs.clear();

    	for (int i = 0 ; i < n ; i++)
    	{
    	    songs.push_back(Song::randInit(10));
    	}
    }

    void playPopulation()
    {
    	int cnt = 1;
    	for (auto &a : songs)
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
    shader_t *_shader;
    std::mutex render_mutex;

    void evaluate() // This is our most complicated function.
    {
        std::cout << "Evaluating\n";
    	int cnt = 0;
    	for (auto &a : songs)
    	{
	    std::cout << "Playing song " << cnt << "\n";
            last_played = cnt;
    	    a.play();

    	    int score = cnt;
    	    // printf("Grade for song nº %d\n", cnt);
    	    // scanf("%d", &score);

    	    if (score > best_score)
    	    {
        		best_score = score;
        		best_idx = cnt;
    	    }
	        a.score = score;
	        cnt++;
    	}
        last_played = -1;
    }

    void draw_last_played()
    {
        if (last_played >= 0)
        {
//            std::cout << "last_played: " << last_played << "\n";
	    render_mutex.lock(); 
            if (!songs[last_played].is_gl_ok) songs[last_played].initGL();
	    songs[last_played].render(_shader);
	    render_mutex.unlock();
        }
    }

    float fitness(Song &a)
    {
    }

    void elitism()
    {
        std::cout << "Elitism happening!\n";

        for (int i = 0 ; i < songs.size() ; i++)
        {
            if (i == best_idx) continue;
	    render_mutex.lock();
            songs[i] = Song::randInit(10);//(songs[best_idx], songs[i]);
	    render_mutex.unlock();
        }
    }
};

#endif

