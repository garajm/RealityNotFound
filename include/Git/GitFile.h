#ifndef GITFILE_H
#define GITFILE_H

#include <QString>
#include <QList>

#include "Git/GitType.h"

namespace Git {

class GitFileDiffBlock;

class GitFile {
public:
    /**
        * GitFile
        * \brief Implicit constructor
        */
    GitFile( void );

    /**
        * \fn GitFile
        * \brief Explicit constructor
        * \param filename - name of Git file
        * \param filepath - path to Git file
        * \param type - type of Git file
     */
    GitFile( QString filename, QString filepath, Git::GitType type);

    /**
        * \fn getFilename
        * \brief Returns filename
     */
    QString getFilename() {
        return filename;
    }

    /**
        * \fn getFilepath
        * \brief Returns filepath
     */
    QString getFilepath() {
        return filepath;
    }

    /**
        * \fn getType
        * \brief Returns type of Git file
     */
    Git::GitType getType() {
        return type;
    }

    QString getTypeAsString();

    /**
        * \fn getGitFileDiffBlocks
        * \brief Returns list of diff blocks in the Git file
     */
    QList<Git::GitFileDiffBlock*> getGitFileDiffBlocks() {
        return gitFileDiffBlocks;
    }

    void addGitFileDiffBlock( Git::GitFileDiffBlock* block ) {
        this->gitFileDiffBlocks.append( block );
    }

    /**
        * \fn setGitFileDiffBlocks
        * \brief Set diff blocks in the Git file
        * \param gitFileDiffBlocks Diff blocks from Git file to be set
     */
    void setGitFileDiffBlocks( QList<Git::GitFileDiffBlock*> gitFileDiffBlocks ) {
        this->gitFileDiffBlocks = gitFileDiffBlocks;
    }

    void printContent();

private:
    /**
        * QString filename
        * \brief Name of Git file
        */
    QString filename;

    /**
        * QString filepath
        * \brief Path to the Git file
        */
    QString filepath;

    /**
        * GitFileType type
        * \brief Type of the Git file in version. Value can be ADDED for newly added file, REMOVED for removed file and MODIFIED for modified file
        */
    Git::GitType type;

    /**
        * QList<GitFileDiffBlock> gitFileDiffBlocks
        * \brief List of diff blocks from the Git file
        */
    QList<Git::GitFileDiffBlock*> gitFileDiffBlocks;
};
}
#endif // GITFILE_H