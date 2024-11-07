void PlaceInIndex(double dx, int ori1, int dy, int ori2, int rx, int cx) {

    int ri = std::max(0, std::min(static_cast<int>(_IndexSize - 1), static_cast<int>(rx)));
    int ci = std::max(0, std::min(static_cast<int>(_IndexSize - 1), static_cast<int>(cx)));


    // Izračunavanje frakcionih delova i težina
    double rfrac = rx - ri;
    double cfrac = cx - ci;
  
    rfrac = std::max(0.0f, std::min(float(rfrac), 1.0f));
    cfrac = std::max(0.0f, std::min(float(cfrac), 1.0f));
  
    double rweight1 = dx * (1.0 - rfrac);
    double rweight2 = dy * (1.0 - rfrac);
    double cweight1 = rweight1 * (1.0 - cfrac);
    doublee cweight2 = rweight2 * (1.0 - cfrac);

    // Pre nego što pristupamo _index, proveravamo da li su ri i ci unutar granica
    if (ri >= 0 && ri < _IndexSize && ci >= 0 && ci < _IndexSize) {
        _index[ri][ci][ori1] = cweight1;
        _index[ri][ci][ori2] = cweight2;
    }

  
}