#pragma once

#include <functional>

class QString;

namespace sparsely
{
    class LabModel;

    class ModelHandler final
    {
      public:
        ModelHandler(LabModel &model);

        void build();
        bool init(const QString &fileName, bool newProject = true);
        bool saveProject(const QString &fileName) const;
        bool loadProject(const QString &fileName);
        void loadAddon(const QString &path);

      private:
        std::reference_wrapper<LabModel> m_Model;
    };
} // namespace sparsely
