#include <vector>
#include <array>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>


namespace three_coloring {

using Graph = std::vector<std::vector<unsigned>>;

}



namespace csp {

struct Configuration {
    unsigned vertex;
    unsigned color;
};


struct Constraint {
    Configuration c1,  c2;
};


class CSP32Instance {
private:
    const unsigned N_;
    std::vector<std::array<std::vector<Configuration>, 3>> constraints_;
    std::vector<std::vector<std::array<bool, 9>>> constraints_matrix_;                                      
    std::vector<std::array<bool, 3>> possible_colors_;


public:
    CSP32Instance(unsigned N, const auto& constraints, const auto& possible_colors)
        : N_(N), constraints_(constraints), possible_colors_(possible_colors) {

        constraints_matrix_.assign(N_, std::vector<std::array<bool, 9>>(N_));
        for (unsigned v = 0; v < N_; ++v)
            for (unsigned color = 0; color < 3; ++color)
                for (const auto& config : constraints_[v][color]) {
                    constraints_matrix_[v][config.vertex][color * 3 + config.color] = true;
                    constraints_matrix_[config.vertex][v][config.color * 3 + color] = true;
                }
    }


    CSP32Instance(const three_coloring::Graph& graph) 
        : N_(graph.size()), constraints_(graph.size()), possible_colors_(graph.size(), {true, true, true}) {
            constraints_matrix_.assign(N_, std::vector<std::array<bool, 9>>(N_));
            for (unsigned v = 0; v < N_; ++v)
                for (unsigned u : graph[v])
                    if (u != v) {
                        constraints_[v][0].push_back({u, 0});
                        constraints_[v][1].push_back({u, 1});
                        constraints_[v][2].push_back({u, 2});
                        constraints_matrix_[u][v][0] = true;
                        constraints_matrix_[u][v][4] = true;
                        constraints_matrix_[u][v][8] = true;
                    }
    }


    bool add_constraint(Constraint c) {
        if (constraints_matrix_[c.c1.vertex][c.c2.vertex][c.c1.color * 3 + c.c2.color])
            return false;


        constraints_[c.c1.vertex][c.c1.color].push_back(c.c2);
        constraints_[c.c2.vertex][c.c2.color].push_back(c.c1);
        
        constraints_matrix_[c.c1.vertex][c.c2.vertex][c.c1.color * 3 + c.c2.color] = true;
        constraints_matrix_[c.c2.vertex][c.c1.vertex][c.c2.color * 3 + c.c1.color] = true;

        return true;
    }


    void remove_last_constraint(Constraint c) {
        if (!constraints_matrix_[c.c1.vertex][c.c2.vertex][c.c1.color * 3 + c.c2.color])
            return;


        constraints_matrix_[c.c1.vertex][c.c2.vertex][c.c1.color * 3 + c.c2.color] = false;
        constraints_matrix_[c.c2.vertex][c.c1.vertex][c.c2.color * 3 + c.c1.color] = false;

        constraints_[c.c1.vertex][c.c1.color].resize(constraints_[c.c1.vertex][c.c1.color].size() - 1);
        constraints_[c.c2.vertex][c.c2.color].resize(constraints_[c.c2.vertex][c.c2.color].size() - 1);
    }


    const auto& get_constraints(Configuration c) const {
        return constraints_[c.vertex][c.color];
    }


    unsigned get_possible_colors(unsigned v) const {
        return possible_colors_[v][0] + (possible_colors_[v][1] << 1) + (possible_colors_[v][2] << 2);
    }

    bool is_color_possible(Configuration c) const {
        return possible_colors_[c.vertex][c.color];
    }

    unsigned get_possible_colors_amount(unsigned v) const {
        unsigned colors = get_possible_colors(v);
        if (colors == 7)
            return 3;
        if (!colors)
            return 0;
        if (colors == 1 || colors == 2 || colors == 4)
            return 1;
        return 2;
    }


    unsigned vertices_number() const {
        return N_;
    }

    void prohibit_color(Configuration c) {
        possible_colors_[c.vertex][c.color] = false;
    }

    void permit_color(Configuration c) {
        possible_colors_[c.vertex][c.color] = true;
    }
};


namespace details__ {


void dump(CSP32Instance& csp, std::vector<bool> is_deleted) {
    printf("==========================================\n");
    for (unsigned v = 0; v < csp.vertices_number(); ++v)
        for (unsigned c = 0; c < 3; ++c)
            if (!is_deleted[v] && csp.is_color_possible({v, c})) {
                printf("v = %u, color = %u, possible = %u\nconstraints = {\n", v, c, csp.get_possible_colors_amount(v));
                for (auto& constraint : csp.get_constraints({v, c}))
                    if (!is_deleted[constraint.vertex] && csp.is_color_possible(constraint))
                        printf("  u = %u, color = %u\n", constraint.vertex, constraint.color);
                printf("}\n");
            }
    printf("==========================================\n");
}

// check if there are any constraints with __v__; if yes, stores it in __constr__
bool has_constraint_3_possible_colors(const CSP32Instance& csp, unsigned v, std::vector<bool>& is_deleted, Constraint& constr) {
    for (unsigned color = 0; color < 3; ++color)
        if (csp.is_color_possible({v, color}))
            for (Configuration conf : csp.get_constraints({v, color}))
                if (!is_deleted[conf.vertex] && csp.is_color_possible(conf) && 
                    csp.get_possible_colors_amount(conf.vertex) == 3) {
                    constr.c1 = {v, color};
                    constr.c2 = conf;
                    return true;
                } 
    return false;
}

// check if there is any constraints with __v__
bool has_constraint(const CSP32Instance& csp, unsigned v, std::vector<bool>& is_deleted) {
    for (unsigned color = 0; color < 3; ++color)
        if (csp.is_color_possible({v, color}))
            for (Configuration conf : csp.get_constraints({v, color}))
                if (!is_deleted[conf.vertex] && csp.is_color_possible(conf))
                    return true;
    return false;
}



// removes __v__ from __csp__, possibly adding new constraints and prohibiting some colors for some vertices
void reduce(CSP32Instance& csp, unsigned v, std::vector<bool>& is_deleted, unsigned& alive) {
    is_deleted[v] = true;
    --alive;
    if (csp.get_possible_colors_amount(v) == 1) {
        unsigned colors = csp.get_possible_colors(v);
        unsigned color = 0;
        while (!((1 << color) & colors))
            ++color;
        auto& constraints = csp.get_constraints({v, color});
        for (auto& config : constraints)
            if (!is_deleted[config.vertex] && csp.is_color_possible(config))
                csp.prohibit_color(config);
    }
    else if (csp.get_possible_colors_amount(v) == 2) {
        unsigned colors = csp.get_possible_colors(v);
        unsigned color1, color2;
        if (colors & 1) {
            color1 = 0;
            color2 = colors & 2 ? 1 : 2;
        }
        else {
            color1 = 1;
            color2 = 2;
        }

        auto& constraints1 = csp.get_constraints({v, color1});
        auto& constraints2 = csp.get_constraints({v, color2});

        for (auto& config1 : constraints1)
            for (auto& config2 : constraints2)
                if (!csp.is_color_possible(config1) || !csp.is_color_possible(config2) || is_deleted[config1.vertex] || is_deleted[config2.vertex])
                    continue;
                else if (config1.vertex != config2.vertex)
                    csp.add_constraint({config1, config2});
                else if (config1.color == config2.color && csp.is_color_possible(config1))
                    csp.prohibit_color(config1);
    }
    else if (csp.get_possible_colors_amount(v) == 3 && !has_constraint(csp, v, is_deleted)) {
        // is_deleted[v] = true;
    }
}



void prohibit_colors_by_type(CSP32Instance& csp, Constraint constr, unsigned type) {
    if (type == 0) {
        csp.prohibit_color({constr.c1.vertex, (constr.c1.color + 2) % 3});
        csp.prohibit_color({constr.c2.vertex, constr.c2.color});
    }
    else if (type == 1) {
        csp.prohibit_color({constr.c1.vertex, (constr.c1.color + 1) % 3});
        csp.prohibit_color({constr.c2.vertex, constr.c2.color});
    }
    else if (type == 2) {
        csp.prohibit_color({constr.c2.vertex, (constr.c2.color + 2) % 3});
        csp.prohibit_color({constr.c1.vertex, constr.c1.color});
    }
    else if (type == 3) {
        csp.prohibit_color({constr.c2.vertex, (constr.c2.color + 1) % 3});
        csp.prohibit_color({constr.c1.vertex, constr.c1.color});
    }
}


bool algo(CSP32Instance& csp,
          std::vector<bool>& is_deleted, unsigned& alive) {

    // dump(csp, is_deleted);

    for (unsigned v = 0; v < csp.vertices_number(); ++v) {
        if (is_deleted[v])
            continue;
        Constraint constr;
        if (csp.get_possible_colors_amount(v) == 3 && has_constraint_3_possible_colors(csp, v, is_deleted, constr)) {
            unsigned type = rand() % 4;
            prohibit_colors_by_type(csp, constr, type);
            reduce(csp, v, is_deleted, alive);
            return algo(csp, is_deleted, alive);
        }
        else if (!csp.get_possible_colors_amount(v))
            return false;
        else if (csp.get_possible_colors_amount(v) < 3 || !has_constraint(csp, v, is_deleted)) {
            reduce(csp, v, is_deleted, alive);
            return algo(csp, is_deleted, alive);
        }
    }
    return !alive;
}


} //namespace details__



bool solve(const CSP32Instance& csp) {
    std::vector<bool> is_deleted(csp.vertices_number());
    unsigned alive = csp.vertices_number();
    
    for (unsigned i = 0; i < pow(2.0, csp.vertices_number() / 2); ++i) {
        is_deleted.assign(csp.vertices_number(), false);
        alive = csp.vertices_number();
        
        auto utility = csp;

        if ( details__::algo(utility, is_deleted, alive))
            return true;
    }
    
    return false;
}


} //namespace csp



namespace three_coloring {

bool is_three_colorable(const Graph& graph) {
    csp::CSP32Instance csp(graph);
    return csp::solve(csp);
}

}
