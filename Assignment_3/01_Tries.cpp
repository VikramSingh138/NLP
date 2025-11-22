#include <cmath>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>

using namespace std;

struct Node
{
    unordered_map<char, Node *> ch;
    int cnt = 0; // frequency count for this node
};

const int BRANCH_THRESHOLD = 15; // minimum branching 

void insert_word(Node *root, const string &w)
{
    Node *n = root;
    n->cnt++;
    for (char c : w)
    {
        if (n->ch[c]) {
            n = n->ch[c];   // child already exists
        } else {
            n->ch[c] = new Node();  // create a new child node
            n = n->ch[c];           
        }
        n->cnt++;
    }
}

// Returns (best_split_index, score, support_count)
tuple<int, double, int> best_split(Node *root, const string &w)
{
    Node *n = root;
    int best_i = -1;
    double best_score = 0.0;
    int best_support = 0;
    for (int i = 0; i < (int)w.size(); ++i)
    {
        char c = w[i];
        if (!n->ch.count(c))
            break;
        n = n->ch[c];
        int branching = (int)n->ch.size();
        if (branching < BRANCH_THRESHOLD)
            continue;

        int max_child = 0;
        for (auto &p : n->ch)
            max_child = max(max_child, p.second->cnt);

        if (n->cnt <= 0) // avoid divide by zero
            continue;

        double frac = 1.0 - double(max_child) / double(n->cnt); // fraction not dominated by one child
        double score = frac * branching;

        // prefer later split if scores are nearly equal
        if (score > best_score + 1e-9 || (fabs(score - best_score) < 1e-9 && i > best_i))
        {
            best_i = i;
            best_score = score;
            best_support = n->cnt;
        }
    }
    return {best_i, best_score, best_support};
}

bool looks_common_suffix(const string &s, string &mapped_stem, string &suf)
{
    // Handle common English suffixes
    if (s.size() > 3 && s.substr(s.size() - 3) == "ies")
    {
        mapped_stem = s.substr(0, s.size() - 3) + "y";
        suf = "ies";
        return true;
    }
    if (s.size() > 2 && s.substr(s.size() - 2) == "es")
    {
        mapped_stem = s.substr(0, s.size() - 2);
        suf = "es";
        return true;
    }
    if (s.size() > 1 && s.back() == 's')
    {
        mapped_stem = s.substr(0, s.size() - 1);
        suf = "s";
        return true;
    }
    if (s.size() > 3 && s.substr(s.size() - 3) == "ing")
    {
        mapped_stem = s.substr(0, s.size() - 3);
        suf = "ing";
        return true;
    }
    if (s.size() > 3 && s.substr(s.size() - 4) == "tion")
    {
        mapped_stem = s.substr(0, s.size() - 4);
        suf = "tion";
        return true;
    }
    return false;
}

int main()
{
    string path = "brown_nouns.txt";
    ifstream fin(path);

    vector<string> words;
    string line;
    while (getline(fin, line))
    {
        for (auto &c : line)
            c = tolower(c);
        words.push_back(line);
    }

    Node *pref = new Node(), *suf = new Node();
    for (auto &w : words)
    {
        insert_word(pref, w);
        string r = w;
        reverse(r.begin(), r.end());
        insert_word(suf, r);
    }

    ofstream pref_ofs("prefix_out.txt"), suf_ofs("suffix_out.txt");
    int pref_count = 0, suf_count = 0;
    double pref_score_sum = 0, suf_score_sum = 0;

    for (auto &w : words)
    {
        // Check prefix split
        auto tup_pref = best_split(pref, w);
        int ip = std::get<0>(tup_pref);
        double sp = std::get<1>(tup_pref);
        int support_p = std::get<2>(tup_pref);
        string stem_p = "", sfx_p = "";
        double score_p = sp;
        int support_pref = support_p;

        if (ip != -1)
        {
            stem_p = w.substr(0, ip + 1);
            sfx_p = w.substr(ip + 1);
            if (stem_p.size() < 2 || sfx_p.empty())
            {
                stem_p = w;
                sfx_p = "";
                score_p = 0;
                support_pref = 0;
            }
        }

        // Check suffix split
        string rw = w;
        reverse(rw.begin(), rw.end());
        auto tup_suf = best_split(suf, rw);
        int ir = std::get<0>(tup_suf);
        double sr = std::get<1>(tup_suf);
        int support_s = std::get<2>(tup_suf);
        string stem_s = "", sfx_s = "";
        double score_s = sr;
        int support_suf = support_s;

        if (ir != -1)
        {
            string rev = rw.substr(0, ir + 1);
            sfx_s = rev;
            reverse(sfx_s.begin(), sfx_s.end());
            stem_s = w.substr(0, w.size() - sfx_s.size());
            if (stem_s.size() < 2 || sfx_s.empty())
            {
                stem_s = w;
                sfx_s = "";
                score_s = 0;
                support_suf = 0;
            }
        }

        // Write prefix output
        if (!sfx_p.empty())
        {
            pref_ofs << w << "=" << stem_p << "+" << sfx_p << "  # score=" << score_p << " support=" << support_pref << "\n";
            pref_count++;
            pref_score_sum += score_p;
        }
        else
            pref_ofs << w << "=" << w << "+  # nosplit\n";

        // Write suffix output
        if (!sfx_s.empty())
        {
            suf_ofs << w << "=" << stem_s << "+" << sfx_s << "  # score=" << score_s << " support=" << support_suf << "\n";
            suf_count++;
            suf_score_sum += score_s;
        }
        else
            suf_ofs << w << "=" << w << "+  # nosplit\n";
    }

    pref_ofs.close();
    suf_ofs.close();

    // Decide whether prefix or suffix splitting works better
    string winner = "prefix";
    if (suf_count > pref_count)
        winner = "suffix";
    else if (suf_count == pref_count)
    {
        if (suf_score_sum > pref_score_sum)
            winner = "suffix";
    }

    // Write final chosen file
    string chosen_fname = (winner == "prefix") ? "prefix_out.txt" : "suffix_out.txt";
    ifstream chosen_in(chosen_fname);
    ofstream final_ofs("trie_q1_output.txt");
    string l;
    while (getline(chosen_in, l))
        final_ofs << l << "\n";
    final_ofs.close();

    cerr << "written prefix_out=" << pref_count << " suffix_out=" << suf_count << " winner=" << winner << "\n";
}
