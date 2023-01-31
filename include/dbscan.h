//
// Created by Nezha on 2016/12/15.
//

/**
 * REAMME *
 * @author: ZJ Jiang (Nezha)
 * @github: https://github.com/CallmeNezha/SimpleDBSCAN
 * @describe: This is a simple DBSCAN clustering method implement
 */

// Option brute force or use kdtree(by default)
#define BRUTEFORCE false
#ifndef __DBSCAN_H__
#define __DBSCAN_H__

#include "gp/signals/event_trigger.hpp"

#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <vector>

#if !BRUTEFORCE
/*
 * @author: Scott Deming, John Tsiombikas
 * @github:https://github.com/sdeming/kdtree
 */
#include "kdtree.h"
#endif

// typedef unsigned int int32_t;

struct vec3f
{
    float data[3];
    float operator[](int idx) const
    {
        return data[idx];
    }
};

//! type T must be a vector-like container and MUST SUPPORT operator[] for
//! iteration Float can be float ,double ,int or any other number type but MUST
//! SUPPORT implicitly convert to double type
template<typename T, typename Float>
class DBSCAN final
{
    enum ERROR_TYPE
    {
        SUCCESS = 0,
        FAILED,
        COUNT
    };

    using TVector = std::vector<T>;
    using DistanceFunc = std::function<Float(const T &, const T &)>;

public:
    DBSCAN()
    {
    }
    ~DBSCAN()
    {
    }

    /**
   * @describe: Run DBSCAN clustering alogrithm
   * @param: V {std::vector<T>} : data
   * @param: dim {unsigned int} : dimension of T (a vector-like struct)
   * @param: eps {Float} : epsilon or in other words, radian
   * @param: min {unsigned int} : minimal number of points in epsilon radian,
   * then the point is cluster core point
   * @param: disfunc {DistanceFunc} : !!!! only used in bruteforce mode.
   * Distance function recall. Euclidian distance is recommanded, but you can
   * replace it by any metric measurement function
   * @param: signals {gp::signals::Signals} : Callback to propagate signals across workflow.
   * @usage: Object.Run() and get the cluster and noise indices from
   * this->Clusters & this->Noise.
   * @pitfall: If you set big eps(search range) and huge density V, then kdtree
   * will be a bottleneck of performance
   * @pitfall: You MUST ensure the data's identicality (TVector* V) during
   * Run(), because DBSCAN just use the reference of data passed in.
   * @TODO: customize kdtree algorithm or rewrite it ,stop further searching
   * when minimal number which indicates cluster core point condition is
   * satisfied
   */
    int Run(
        TVector *V, const int32_t dim, const Float eps, const int32_t min, const DistanceFunc &disfunc = [](const T &t1, const T &t2) -> Float { return 0; },
        const gp::signals::Signals &signals = {});

private:
    std::vector<int32_t> regionQuery(const int32_t pid, const gp::signals::Signals &signals) const;
    void addToCluster(const int32_t pid, const int32_t cid);
    void expandCluster(const int32_t cid, const std::vector<int32_t> &neighbors, const gp::signals::Signals &signals);
    void addToBorderSet(const int32_t pid)
    {
        this->_borderset.insert(pid);
    }
    void addToBorderSet(const std::vector<int32_t> &pids)
    {
        for (int32_t pid : pids)
        {
            this->_borderset.insert(pid);
        }
    }
    bool isInBorderSet(const int32_t pid) const
    {
        return this->_borderset.end() != this->_borderset.find(pid);
    }
    void buildKdtree(const TVector *V, const gp::signals::Signals &signals);
    void destroyKdtree();

public:
    std::vector<std::vector<int32_t>> Clusters;
    std::vector<int32_t> Noise;

private:
    // temporary variables used during computation
    std::vector<bool> _visited;
    std::vector<bool> _assigned;
    std::set<int32_t> _borderset;
    int32_t _datalen;
    int32_t _minpts;
    Float _epsilon;
    int32_t _datadim;

    DistanceFunc _disfunc;
#if !BRUTEFORCE
    kdtree *_kdtree;
#endif //! BRUTEFORCE

    std::vector<T> *_data; // Not owner, just holder, no responsible for deallocate
};

template<typename T, typename Float>
int DBSCAN<T, Float>::Run(TVector *V, const int32_t dim, const Float eps, const int32_t min, const DistanceFunc &disfunc, const gp::signals::Signals &signals)
{
    // Validate
    if (V->size() < 1)
        return ERROR_TYPE::FAILED;
    if (dim < 1)
        return ERROR_TYPE::FAILED;
    if (min < 1)
        return ERROR_TYPE::FAILED;

    // initialization
    this->_datalen = (int32_t)V->size();
    this->_visited = std::vector<bool>(this->_datalen, false);
    this->_assigned = std::vector<bool>(this->_datalen, false);
    this->Clusters.clear();
    this->Noise.clear();
    this->_minpts = min;
    this->_data = V;
    this->_disfunc = disfunc;
    this->_epsilon = eps;
    this->_datadim = dim;

#if BRUTEFORCE
#else
    this->buildKdtree(this->_data, signals);
#endif // !BRUTEFORCE

    for (int32_t pid = 0; pid < this->_datalen; ++pid)
    {
        gp::signals::checkStopSignalAndThrow(signals);
        // Check if point forms a cluster
        this->_borderset.clear();
        if (!this->_visited[pid])
        {
            this->_visited[pid] = true;

            // Outliner it maybe noise or on the border of one cluster.
            const std::vector<int32_t> neightbors = this->regionQuery(pid, signals);
            if (neightbors.size() < this->_minpts)
            {
                continue;
            }
            else
            {
                int32_t cid = (int32_t)this->Clusters.size();
                this->Clusters.push_back(std::vector<int32_t>());
                // first blood
                this->addToBorderSet(pid);
                this->addToCluster(pid, cid);
                this->expandCluster(cid, neightbors, signals);
            }
        }
    }

    for (int32_t pid = 0; pid < this->_datalen; ++pid)
    {
        gp::signals::checkStopSignalAndThrow(signals);
        if (!this->_assigned[pid])
        {
            this->Noise.push_back(pid);
        }
    }

#if BRUTEFORCE
#else
    this->destroyKdtree();
#endif // !BRUTEFORCE

    return ERROR_TYPE::SUCCESS;
}

template<typename T, typename Float>
void DBSCAN<T, Float>::destroyKdtree()
{
    kd_free(this->_kdtree);
}

template<typename T, typename Float>
void DBSCAN<T, Float>::buildKdtree(const TVector *V, const gp::signals::Signals &signals)
{
    this->_kdtree = kd_create((int)this->_datadim);
    std::unique_ptr<double[]> v(new double[this->_datadim]);
    for (int32_t r = 0; r < this->_datalen; ++r)
    {
        gp::signals::checkStopSignalAndThrow(signals);
        // kdtree only support double type
        for (int32_t c = 0; c < this->_datadim; ++c)
        {
            v[c] = (double)(*V)[r][c];
        }
        kd_insert(this->_kdtree, v.get(), (void *)&(*V)[r]);
    }
}

template<typename T, typename Float>
std::vector<int32_t> DBSCAN<T, Float>::regionQuery(const int32_t pid, const gp::signals::Signals &signals) const
{
    std::vector<int32_t> neighbors;

#if BRUTEFORCE // brute force  O(n^2)
    for (int32_t i = 0; i < this->_data->size(); ++i)
        if (i != pid && this->_disfunc((*this->_data)[pid], (*this->_data)[i]) < this->_epsilon)
            neighbors.push_back(i);
#else // kdtree
    std::unique_ptr<double[]> v(new double[this->_datadim]);
    for (int32_t c = 0; c < this->_datadim; ++c)
    {
        v[c] = (double)((*this->_data)[pid][c]);
    }

    kdres *presults = kd_nearest_range(this->_kdtree, v.get(), this->_epsilon);
    while (!kd_res_end(presults))
    {
        gp::signals::checkStopSignalAndThrow(signals);
        /* get the data and position of the current result item */
        T *pch = (T *)kd_res_item(presults, v.get());
        int32_t pnpid = (int32_t)(pch - &(*this->_data)[0]);
        if (pid != pnpid)
            neighbors.push_back(pnpid);
        /* go to the next entry */
        kd_res_next(presults);
    }
    kd_res_free(presults);

#endif // !BRUTEFORCE

    return neighbors;
}

template<typename T, typename Float>
void DBSCAN<T, Float>::expandCluster(const int32_t cid, const std::vector<int32_t> &neighbors, const gp::signals::Signals &signals)
{
    std::queue<int32_t> border; // it has unvisited , visited unassigned pts.
                                // visited assigned will not appear
    for (int32_t pid : neighbors)
        border.push(pid);
    this->addToBorderSet(neighbors);

    while (border.size() > 0)
    {
        gp::signals::checkStopSignalAndThrow(signals);
        const int32_t pid = border.front();
        border.pop();

        if (!this->_visited[pid])
        {
            // not been visited, great! , hurry to mark it visited
            this->_visited[pid] = true;
            const std::vector<int32_t> pidneighbors = this->regionQuery(pid, signals);

            // Core point, the neighbors will be expanded
            if (pidneighbors.size() >= this->_minpts)
            {
                this->addToCluster(pid, cid);
                for (int32_t pidnid : pidneighbors)
                {
                    if (!this->isInBorderSet(pidnid))
                    {
                        border.push(pidnid);
                        this->addToBorderSet(pidnid);
                    }
                }
            }
        }
    }
}

template<typename T, typename Float>
void DBSCAN<T, Float>::addToCluster(const int32_t pid, const int32_t cid)
{
    this->Clusters[cid].push_back(pid);
    this->_assigned[pid] = true;
}

#endif //__DBSCAN_H__
