#include "beep.hpp"
#include <vector>

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
    }

    static Song randInit(int n)
    {
	Song gen;

	for (int i = 0 ; i < n ; i++)
	{
	    gen.notes.push_back({static_cast<double>(rand() % 1780), 100 + rand() % 800});
	}

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

    void evaluate()
    {
	int cnt = 0;
	for (Song &a : songs)
	{
	    a.play();
	    int score;
	    printf("Grade for song nº %d\n", cnt);
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

    void elitism()
    {
	for (int i = 0 ; i < songs.size() ; i++)
	{
	    if (i == best_idx) continue;
	    songs[i] = Song::genFromParents(songs[best_idx], songs[i]);
	}
    }
};

