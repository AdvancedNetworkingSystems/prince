namespace outops {
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s) {
        using namespace boost::spirit::karma;
        os << format("(" << (auto_ % "\n  ") << ")", s);
        os << endl;
    }
}

namespace graphext {
    template <typename Container>
    void id_of_vertices(const Graph& g, const Container& container, std::set<std::string>& r) {
        /*
        ** Find id for a vec
        */
        for (typename Container::const_iterator ci = container.begin(); ci != container.end(); ++ci) {
            r.insert(g[*ci].id);
        }
    }
}

