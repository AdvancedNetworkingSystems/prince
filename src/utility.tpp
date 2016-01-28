namespace outops
{
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& s)
    {
        using namespace boost::spirit::karma;
        os << format("(" << (auto_ % "\n  ") << ")", s);
        os << endl;
    }
}

namespace printhelper {
    template <typename T1, typename T2>
    void for_map(const std::map<T1, T2> m) {
        // similar to cout<< in namespace outops
        cout << "printhelper - for_map\n";
        typename std::map<T1, T2>::const_iterator iter;
        for (iter = m.begin(); iter != m.end(); ++iter) {
            cout << (*iter).first << ": " << (*iter).second;
        }
    }
}
namespace graphext {
    template <typename Container>
    void id_of_some_vertices(const Graph& g, const Container& container, std::set<std::string>& r)
    {
        /*
        ** Find id for a vec
        */
        for (typename Container::const_iterator ci = container.begin(); ci != container.end(); ++ci) {
            r.insert(g[*ci].id);
        }
    }
}

namespace setops {
    // From http://stackoverflow.com/questions/8175933/to-compare-two-boost-graph-having-same-vertices
    template <typename T> std::set<T> operator-(const std::set<T>& a, const std::set<T>& b)
    {
        std::set<T> r;
        std::set_difference(
                a.begin(), a.end(),
                b.begin(), b.end(),
                std::inserter(r, r.end()));

        return r;
    }

    template <typename T> std::set<T> operator/(const std::set<T>& a, const std::set<T>& b)
    {
        std::set<T> r;
        std::set_intersection(
                a.begin(), a.end(),
                b.begin(), b.end(),
                std::inserter(r, r.end()));

        return r;
    }
}

namespace stdhelper {
    template <typename T1, typename T2>
    bool exists(const std::map<T1, T2>& c, const T1& key) {
        return (c.count(key) > 0);
    }

    template <typename T>
    bool exists(const std::set<T>& c, const T& key) {
        return (c.count(key) > 0);
    }
}

