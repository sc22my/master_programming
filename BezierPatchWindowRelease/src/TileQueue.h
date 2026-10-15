// Organises the screen into tiles to draw points
// Each tile is processed independantly

#include "Homogeneous4.h"
#include "RGBAValue.h"

#include <cstdint>
#include <vector>
#include <omp.h>

// Forward declares
class RGBAImage;

#ifndef _TILEQUEUE_H
#define _TILEQUEUE_H

// The tile size in pixels
#define TILE_SIZE 16

class RenderTile {
public:
    // Holds a framebuffer coordinate
    struct FBCoordinate {
        uint16_t fbx;
        uint16_t fby;
        float depth; // depth value in ndc range: [-1, 1]
    };

    RenderTile();
    ~RenderTile();

    void Clear() { m_Positions.clear(); m_Colors.clear(); }

    // Point queue for individual tile
    inline void QueuePoint(FBCoordinate coordinate, RGBAValue color){
        // Lock this tile while updating the vectors
        omp_set_lock(&m_Lock);

        m_Positions.push_back(coordinate);
        m_Colors.push_back(color);

        // Release lock
        omp_unset_lock(&m_Lock);
    }

    // For metrics
    unsigned long GetNumQueuedPoints() const { return m_Positions.size(); }

    // draws queued points into framebuffer with offsets x and y
    void DrawPoints(RGBAImage* fb, int x, int y);
private:
    // The amount of space to reserve for points initially
    // For some reason when the vector is reallocated it causes a crash.
    // So make it large enough from the start and hope for the best :p
    static constexpr unsigned int m_QueueReserveAmount = 1024;

    // The point queue: structure of arrays
    // Positions are in framebuffer coordinates, one value for x, and one for y
    // Using uint16 as we prob dont have width/height > 64k, so we can fit more points into cache
    std::vector<FBCoordinate> m_Positions;
    std::vector<RGBAValue> m_Colors;

    // lock for queueing points
    omp_lock_t m_Lock;
};

// Manages tiles. Starts off 0x0 so Resize() will need to be called
class TileManager {
public:
    void Resize(unsigned int width, unsigned int height);

    // This is ok to be called in parallel
    inline void QueuePoint(Homogeneous4 pos, RGBAValue color) {
        // Get Homogenious position
        if (pos.w != 0) {
            pos = pos / pos.w;
        }

        // find framebuffer coordinates
        uint16_t fbx = (pos.x + 1) * (m_Width >> 1);
        uint16_t fby = (pos.y + 1) * (m_Height >> 1);

        // Clip point
        if (fbx < 0 || fbx >= m_Width) return;
        if (fby < 0 || fby >= m_Height) return;
        if (pos.z <= -1 || pos.z >= 1) return;

        // Find tile
        int tilex = fbx / TILE_SIZE;
        int tiley = fby / TILE_SIZE;
        int tile_index = tiley * m_NumTilesX + tilex;

        m_Tiles[tile_index].QueuePoint(RenderTile::FBCoordinate { fbx, fby, pos.z }, color);
    }

    // Draws points and clears the queue.
    // Multithreaded function, so must be called from main thread
    void Draw(RGBAImage* fb);

    // Get how many points we drew after Draw();
    unsigned long GetNumPointsDrawn() const { return m_PointsDrawn; }
private:
    // Width and height in pixels
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;

    // Number of tiles we have x and y
    unsigned int m_NumTilesX = 0;
    unsigned int m_NumTilesY = 0;

    // The render tiles
    std::vector<RenderTile> m_Tiles;

    // How many points we drew
    unsigned int m_PointsDrawn = 0;
};

#endif