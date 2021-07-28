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

	gen.compute_song_metrics();

        return gen;
        //exit(EXIT_SUCCESS);
    }

    Song(int n)
    {
        _triangle_count = 0;
        is_gl_ok = false;

        for (int i = 0 ; i < n ; i++)
        {
            notes.push_back({(rand() % KEYS), 100 + rand() % 800});
        }
	compute_song_metrics();
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
	compute_song_metrics();
    }


    static Song randInit(int n)
    {
        Song gen;

        for (int i = 0 ; i < n ; i++)
        {
            gen.notes.push_back({(rand() % KEYS), 100 + rand() % 800});
        }

        //gen.initGL();
	gen.compute_song_metrics();
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

	// TODO: Create a logger or something like that
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

    float interval_values_mean;
    float interval_values_var;

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
        float last_x = -1.0, t_x, t_y, dy;

        for (Note &a_note : notes)
        {
            const float time_offset = (float) a_note.duration / 15000.0f;

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

            last_x += time_offset;// + 0.005;
        }
    }

    // Here, following the reference paper, we will establish values for each
    // type of interval.
    // perfect consonants unison, perfect fourth, 
    // perfect fifth, octave 1 1 
    // imperfect 
    // consonants 
    // minor and major thirds 
    // and sixths 2 3 
    // seconds minor and major seconds 3 1 
    // sevenths minor and major sevenths 3 3 
    // intervals greater 
    // than octave 
    // all intervals greater than 
    // octave 5 5 
    //
    void compute_song_metrics()
    {
	// Intervals are between two notes.
	const int interval_n = notes.size()-1;
	int *intervals = new int[interval_n];

	for (int i = 1 ; i < notes.size() ; i++)
	{
	    const int interval = notes[i].note_n - notes[i-1].note_n;
	    int val;

	    if (interval == 0 ||
	        interval == 12 ||
		interval == 5 ||
		interval == 7) val = 1;
	    else if (interval == 3 ||
		     interval == 4 ||
		     interval == 8 ||
		     interval == 9) val = 2;
	    else if (interval == 1 ||
		     interval == 2 ||
		     interval == 10 ||
		     interval == 11) val = 3;
	    else if (interval > 12) val = 5;

	    intervals[i-1] = val;
	}

	float avg = 0.0;
	float avg_square = 0.0;

	// First and second distribution moments.
	for (int i = 0 ; i < interval_n ; i++)
	{
	    avg += (float)intervals[i];
	    avg_square += (float)intervals[i] + (float)intervals[i];
	}

	avg /= (float)interval_n;
	avg_square /= (float)interval_n;

	interval_values_mean = avg;
	interval_values_var = avg_square - (avg * avg); // Variance def.

	delete[] intervals;
    }
};

class Population
{
public:
    Population(int n, Song &reference_song) : _reference_song(reference_song)
    {
        songs.clear();

    	for (int i = 0 ; i < n ; i++)
    	{
    	    songs.push_back(Song::randInit(10));
    	}
    }

    void playAll()
    {
	playing_ref = true;
	_reference_song.play();
	playing_ref = false;

	int cnt = 0;
    	for (auto &a : songs)
    	{
    	    #ifdef DEBUG
    	    printf("Playing song nº %d\n", cnt);
    	    #endif

	    last_played = cnt++;
    	    a.play();
    	}
    }

    std::vector<Song> songs;
    int iter_n;
    float best_score = 0.0;
    int best_idx = 0;
    int last_played = -1;
    shader_t *_shader;
    std::mutex render_mutex;
    Song &_reference_song; // Like, literally.
    bool playing_ref = false;

    void evaluate() // This is our most complicated function.
    {
        std::cout << "Evaluating\n";
    	int cnt = 0;
    	for (auto &a : songs)
    	{
	    //std::cout << "Playing song " << cnt << "\n";
            //last_played = cnt;
    	    //a.play();

    	    const float score = fitness(a);
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
	if (playing_ref)
	{
	    render_mutex.lock();
	    _reference_song.render(_shader);
	    render_mutex.unlock();
	}
        else if (last_played >= 0)
        {
	    // std::cout << "last_played: " << last_played << "\n";
	    render_mutex.lock();
            if (!songs[last_played].is_gl_ok) songs[last_played].initGL();
	    songs[last_played].render(_shader);
	    render_mutex.unlock();
        }
    }

    // Compare the intervals to our reference song.
    float fitness(Song &a)
    {
	float dist = fabs(a.interval_values_mean - _reference_song.interval_values_mean);
	dist += fabs(a.interval_values_var - _reference_song.interval_values_var);

	return (float)1.0/dist;
    }

    void elitism()
    {
        std::cout << "Elitism happening!\n";

        for (int i = 0 ; i < songs.size() ; i++)
        {
            if (i == best_idx) continue;
	    render_mutex.lock();
            songs[i] = Song(songs[best_idx], songs[i]);
	    render_mutex.unlock();
        }
    }
};

#endif

