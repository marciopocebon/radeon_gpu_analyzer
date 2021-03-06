//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderOpenGL.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beBackend.h>

// Device info.
#include <DeviceInfoUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// The list of devices not supported by VirtualContext.
static const std::set<std::string>  RGA_OPENGL_DISABLED_DEVICES = {};

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Internally-linked utilities.
static bool GetVirtualContextPath(std::string& virtualContextPath)
{
#ifdef __linux
    virtualContextPath = "VirtualContext";
#elif _WIN64
    virtualContextPath = "x64\\VirtualContext.exe";
#else
    virtualContextPath = "x86\\VirtualContext.exe";
#endif
    return true;
}

beProgramBuilderOpenGL::beProgramBuilderOpenGL()
{

}

beProgramBuilderOpenGL::~beProgramBuilderOpenGL()
{
}

beKA::beStatus beProgramBuilderOpenGL::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(binopts);
    GT_UNREFERENCED_PARAMETER(binary);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetKernelILText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    (void)table;
    return beKA::beStatus_Invalid;
}

// Checks if the required output files are generated by the amdspv.
// Only verifies the files requested in the "options.m_pipelineShaders" name list.
static bool  VerifyVirtualContextOutput(const OpenGLOptions& options)
{
    bool  ret = true;
    if (options.m_isAmdIsaDisassemblyRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::IsFilePresent(options.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray()));
        assert(ret);
    }
    if (options.m_isIlDisassemblyRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_computeShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_geometryShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty() || beUtils::IsFilePresent(options.m_ilDisassemblyOutputFiles.m_vertexShader.asASCIICharArray()));
        assert(ret);
    }
    if (ret && options.m_isAmdIsaBinariesRequired)
    {
        ret &= beUtils::IsFilePresent(options.m_programBinaryFile.asASCIICharArray());
        assert(ret);
    }
    if (ret && options.m_isScStatsRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        assert(ret);
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::IsFilePresent(options.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray()));
        assert(ret);
    }

    return ret;
}

beKA::beStatus beProgramBuilderOpenGL::Compile(const OpenGLOptions& glOptions, bool& cancelSignal, bool printCmd, gtString& vcOutput)
{
    GT_UNREFERENCED_PARAMETER(cancelSignal);
    beKA::beStatus ret = beKA::beStatus_SUCCESS;

    // Clear the output buffer if needed.
    if (!vcOutput.isEmpty())
    {
        vcOutput.makeEmpty();
    }

    // Get VC's path.
    std::string vcPath;
    GetVirtualContextPath(vcPath);

    AMDTDeviceInfoUtils* pDeviceInfo = AMDTDeviceInfoUtils::Instance();

    if (pDeviceInfo != nullptr)
    {
        const char VC_CMD_DELIMITER = ';';

        // Build the command for invoking Virtual Context.
        std::stringstream cmd;

        // ISA.
        cmd << vcPath << " \"" << glOptions.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Program binary.
        cmd << glOptions.m_programBinaryFile.asASCIICharArray() << VC_CMD_DELIMITER;

        // Statistics.
        cmd << glOptions.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Target device info.
        cmd << glOptions.m_chipFamily << VC_CMD_DELIMITER << glOptions.m_chipRevision << VC_CMD_DELIMITER;

        // Input shaders.
        cmd << glOptions.m_pipelineShaders.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // An additional delimiter for the version slot.
        cmd << VC_CMD_DELIMITER;

        // IL disassembly output.
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_ilDisassemblyOutputFiles.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << "\"";

        // Build the GL program.
        bool isCompilerOutputRelevant = false;
        beUtils::PrintCmdLine(cmd.str(), printCmd);

        // Workaround for random VirtualContext failures: make 3 attempts with increasing intervals.
        static const unsigned long VC_WAIT_INTERVAL_1 = 2000;
        static const unsigned long VC_WAIT_INTERVAL_2 = 4000;
        bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, vcOutput);
        if (!isLaunchSuccess || vcOutput.isEmpty())
        {
            // First attempt failed, wait and make a second attempt.
            osSleep(VC_WAIT_INTERVAL_1);
            isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, vcOutput);

            // Second attempt failed, wait and make the last attempt.
            if (!isLaunchSuccess || vcOutput.isEmpty())
            {
                osSleep(VC_WAIT_INTERVAL_2);
                isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, vcOutput);
            }
        }
        assert(isLaunchSuccess && !vcOutput.isEmpty());

        if (isLaunchSuccess)
        {
            const gtString VC_ERROR_TOKEN = L"error:";
            gtString vcOutputInLowerCase = vcOutput;
            vcOutputInLowerCase.toLowerCase();

            if (vcOutputInLowerCase.find(VC_ERROR_TOKEN) != -1)
            {
                ret = beKA::beStatus_GLOpenGLBuildError;
                isCompilerOutputRelevant = true;
            }
            else if (!VerifyVirtualContextOutput(glOptions))
            {
                ret = beKA::beStatus_FailedOutputVerification;
            }
        }
        else
        {
            ret = beKA::beStatus_GLOpenGLVirtualContextFailedToLaunch;
        }

        // Clear the output if irrelevant.
        if (!isCompilerOutputRelevant)
        {
            vcOutput.makeEmpty();
        }
    }

    return ret;
}

bool beProgramBuilderOpenGL::GetOpenGLVersion(bool printCmd, gtString& glVersion)
{
    // Get VC's path.
    std::string vcPath;
    GetVirtualContextPath(vcPath);

    // Build the command for invoking Virtual Context.
    std::stringstream cmd;
    cmd << vcPath << " \";;;;;;;;;;;;;;;;;;;;;version;\"";

    // A flag for canceling the operation, we will not use it.
    bool dummyCancelFlag = false;
    beUtils::PrintCmdLine(cmd.str(), printCmd);
    bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), dummyCancelFlag, glVersion);

    return isLaunchSuccess;
}

bool beProgramBuilderOpenGL::GetDeviceGLInfo(const std::string& deviceName, size_t& deviceFamilyId, size_t& deviceRevision) const
{
    bool ret = false;

    // This map will hold the device values as expected by the OpenGL backend.
    static std::map<std::string, std::pair<size_t, size_t>> glBackendValues;
    if (glBackendValues.empty())
    {
        // Fill in the values if that's the first time.
        glBackendValues["Bonaire"] = std::pair<int, int>(120, 20);
        glBackendValues["Bristol Ridge"] = std::pair<int, int>(130, 10);
        glBackendValues["Capeverde"] = std::pair<int, int>(110, 40);
        glBackendValues["Carrizo"] = std::pair<int, int>(130, 1);
        glBackendValues["Fiji"] = std::pair<int, int>(130, 60);
        glBackendValues["Hainan"] = std::pair<int, int>(110, 75);
        glBackendValues["Hawaii"] = std::pair<int, int>(120, 40);
        glBackendValues["Iceland"] = std::pair<int, int>(130, 19);
        glBackendValues["Kalindi"] = std::pair<int, int>(120, 129);
        glBackendValues["Mullins"] = std::pair<int, int>(120, 161);
        glBackendValues["Oland"] = std::pair<int, int>(110, 60);
        glBackendValues["Pitcairn"] = std::pair<int, int>(110, 20);
        glBackendValues["Spectre"] = std::pair<int, int>(120, 1);
        glBackendValues["Spooky"] = std::pair<int, int>(120, 65);
        glBackendValues["Stoney"] = std::pair<int, int>(130, 97);
        glBackendValues["Tahiti"] = std::pair<int, int>(110, 0);
        glBackendValues["Tonga"] = std::pair<int, int>(130, 20);
        glBackendValues["Baffin"] = std::pair<int, int>(130, 91);
        glBackendValues["Ellesmere"] = std::pair<int, int>(130, 89);
        glBackendValues["gfx804"] = std::pair<int, int>(130, 100);
        glBackendValues["gfx900"] = std::pair<int, int>(141, 1);
        glBackendValues["gfx902"] = std::pair<int, int>(141, 27);
        glBackendValues["gfx906"] = std::pair<int, int>(141, 40);
        glBackendValues["gfx1010"] = std::pair<int, int>(143, 1);
        glBackendValues["gfx1012"] = std::pair<int, int>(143, 20);
    }

    // Fetch the relevant value.
    auto deviceIter = glBackendValues.find(deviceName);
    if (deviceIter != glBackendValues.end())
    {
        deviceFamilyId = deviceIter->second.first;
        deviceRevision = deviceIter->second.second;
        ret = true;
    }

    return ret;
}

bool beProgramBuilderOpenGL::GetSupportedDevices(std::set<std::string>& deviceList)
{
    std::vector<GDT_GfxCardInfo> tmpCardList;
    bool ret = beUtils::GetAllGraphicsCards(tmpCardList, deviceList);
    // Remove unsupported devices.
    if (ret)
    {
        for (const std::string& device : RGA_OPENGL_DISABLED_DEVICES)
        {
            deviceList.erase(device);
        }
    }
    return ret;
}

const std::set<std::string>& beProgramBuilderOpenGL::GetDisabledDevices()
{
    return RGA_OPENGL_DISABLED_DEVICES;
}
