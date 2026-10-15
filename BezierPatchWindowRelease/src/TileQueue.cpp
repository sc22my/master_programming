#include "TileQueue.h"

#include "RGBAImage.h"
#include "Sort.h"
#include <cmath>

RenderTile::RenderTile() {
    // Pre allocate some memory for tile
    m_Positions.reserve(m_QueueReserveAmount);
    m_Colors.reserve(m_QueueReserveAmount);

    // Initialise lock
    omp_init_lock(&m_Lock);
}

RenderTile::~RenderTile() {
    // Destroy lock
    omp_destroy_lock(&m_Lock);
}

void RenderTile::DrawPoints(RGBAImage* fb, int x, int y) {
    // Sort points back to front
    QuickSort::Sort(m_Positions, [] (const FBCoordinate& coord) { return -coord.depth; }, m_Colors);

    // Fill in point
    for (int i = 0; i < m_Positions.size(); i++) {
        FBCoordinate pt = m_Positions[i];

        (*fb)[pt.fby][pt.fbx] = m_Colors[i];
    }
}

void TileManager::Resize(unsigned int width, unsigned int height) {
    // Set out own vars
    m_Width = width;
    m_Height = height;

    // Check if we have enough tiles
    m_NumTilesX = std::ceil((float)width / TILE_SIZE);
    m_NumTilesY = std::ceil((float)height / TILE_SIZE);

    // Create new tiles
    if (m_Tiles.size() < m_NumTilesX * m_NumTilesY) {
        m_Tiles.resize(m_NumTilesX * m_NumTilesY);
    }

    // Clear tiles as contents are invalidated
    for (auto& tile : m_Tiles) {
        tile.Clear();
    }
}

void TileManager::Draw(RGBAImage* fb) {
    int numtiles = m_NumTilesX * m_NumTilesY;

    // Clear metrics
    m_PointsDrawn = 0;

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < numtiles; i++) {
        uint16_t y = (i / m_NumTilesX) * TILE_SIZE;
        uint16_t x = (i % m_NumTilesX) * TILE_SIZE;

        // Add to metrics
        #pragma omp atomic
        m_PointsDrawn += m_Tiles.GetNumQueuedPoints();

        m_Tiles[i].DrawPoints(fb, x, y);
    }

    // clear tiles. do it on the main thread
    for (auto& t : m_Tiles) {
        t.Clear();
    }
}