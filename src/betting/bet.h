// Copyright (c) 2018-2020 The Wagerr developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WAGERR_BET_H
#define WAGERR_BET_H

#include "util.h"
#include "chainparams.h"
#include "leveldbwrapper.h"
#include "base58.h"

#include <flushablestorage/flushablestorage.h>
#include <boost/variant.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/exception/to_string.hpp>

#define BET_ODDSDIVISOR 10000   // Odds divisor, Facilitates calculations with floating integers.
#define BET_BURNXPERMILLE 60    // Burn promillage

// The supported bet outcome types.
typedef enum OutcomeType {
    moneyLineHomeWin  = 0x01,
    moneyLineAwayWin = 0x02,
    moneyLineDraw = 0x03,
    spreadHome    = 0x04,
    spreadAway    = 0x05,
    totalOver     = 0x06,
    totalUnder    = 0x07
} OutcomeType;

// The supported result types
typedef enum ResultType {
    standardResult = 0x01,
    eventRefund    = 0x02,
    mlRefund       = 0x03,
    spreadsRefund  = 0x04,
    totalsRefund   = 0x05,
} ResultType;

// The supported result types
typedef enum WinnerType {
    homeWin = 0x01,
    awayWin = 0x02,
    push    = 0x03,
} WinnerType;

// The supported betting TX types.
typedef enum BetTxTypes{
    mappingTxType        = 0x01,  // Mapping transaction type identifier.
    plEventTxType        = 0x02,  // Peerless event transaction type identifier.
    plBetTxType          = 0x03,  // Peerless Bet transaction type identifier.
    plResultTxType       = 0x04,  // Peerless Result transaction type identifier.
    plUpdateOddsTxType   = 0x05,  // Peerless update odds transaction type identifier.
    cgEventTxType        = 0x06,  // Chain games event transaction type identifier.
    cgBetTxType          = 0x07,  // Chain games bet transaction type identifier.
    cgResultTxType       = 0x08,  // Chain games result transaction type identifier.
    plSpreadsEventTxType = 0x09,  // Spread odds transaction type identifier.
    plTotalsEventTxType  = 0x0a,  // Totals odds transaction type identifier.
    plEventPatchTxType   = 0x0b,  // Peerless event patch transaction type identifier.
    plParlayBetTxType    = 0x0c,  // Peerless Parlay Bet transaction type identifier.
    qgBetTxType          = 0x0d,  // Quick Games Bet transaction type identifier.
} BetTxTypes;

// The supported mapping TX types.
typedef enum MappingTypes {
    sportMapping      = 0x01,
    roundMapping      = 0x02,
    teamMapping       = 0x03,
    tournamentMapping = 0x04
} MappingTypes;

//
typedef enum PayoutType {
    bettingPayout    = 0x01,
    bettingRefund    = 0x02,
    bettingReward    = 0x03,
    chainGamesPayout = 0x04,
    chainGamesRefund = 0x05,
    chainGamesReward = 0x06,
    quickGamesPayout = 0x07,
    quickGamesRefund = 0x08,
    quickGamesReward = 0x09
} PayoutType;

typedef enum BetResultType {
    betResultUnknown = 0x00,
    betResultWin = 0x01,
    betResultLose = 0x02,
    betResultRefund = 0x03,
} BetResultType;

// Class derived from CTxOut
// nBetValue is NOT serialized, nor is it included in the hash or in comparison functions
class CBetOut : public CTxOut {
    private:

    void Set(const CAmount& nValueIn, CScript scriptPubKeyIn, const CAmount& nBetValueIn = 0, uint32_t nEventIdIn = 0)
    {
        nValue = nValueIn;
        scriptPubKey = scriptPubKeyIn;
        nBetValue = nBetValueIn;
        nEventId = nEventIdIn;
    }

    public:

    CAmount nBetValue;
    uint32_t nEventId;

    CBetOut() : CTxOut() {
        SetNull();
    }

    CBetOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
    {
        Set(nValueIn,scriptPubKeyIn);
    };

    CBetOut(const CAmount& nValueIn, CScript scriptPubKeyIn, const CAmount& nBetValueIn)
    {
        Set(nValueIn,scriptPubKeyIn, nBetValueIn);
    };

    CBetOut(const CAmount& nValueIn, CScript scriptPubKeyIn, const CAmount& nBetValueIn, uint32_t nEventIdIn)
    {
        Set(nValueIn,scriptPubKeyIn, nBetValueIn, nEventIdIn);
    };

    void SetNull() {
        CTxOut::SetNull();
        nBetValue = -1;
        nEventId = -1;
    }

    void SetEmpty() {
        CTxOut::SetEmpty();
        nBetValue = 0;
        nEventId = 0;
    }

    bool IsEmpty() const {
        return CTxOut::IsEmpty() && nEventId == 0;
    }

    inline int CompareTo(const CBetOut& rhs) const
    {
        if (nValue < rhs.nValue)
            return -1;
        if (nValue > rhs.nValue)
            return 1;
        if (scriptPubKey < rhs.scriptPubKey)
            return -1;
        if (scriptPubKey > rhs.scriptPubKey)
            return 1;
        if (nEventId < rhs.nEventId)
            return -1;
        if (nEventId > rhs.nEventId)
            return 1;
        return 0;
    }

    inline bool operator==(const CBetOut& rhs) const { return CompareTo(rhs) == 0; }
    inline bool operator!=(const CBetOut& rhs) const { return CompareTo(rhs) != 0; }
    inline bool operator<=(const CBetOut& rhs) const { return CompareTo(rhs) <= 0; }
    inline bool operator>=(const CBetOut& rhs) const { return CompareTo(rhs) >= 0; }
    inline bool operator<(const CBetOut& rhs) const { return CompareTo(rhs) < 0; }
    inline bool operator>(const CBetOut& rhs) const { return CompareTo(rhs) > 0; }
};

class CPeerlessEvent
{
public:
    uint32_t nEventId = 0;
    uint64_t nStartTime = 0;
    uint32_t nSport = 0;
    uint32_t nTournament = 0;
    uint32_t nStage = 0;
    uint32_t nHomeTeam = 0;
    uint32_t nAwayTeam = 0;
    uint32_t nHomeOdds = 0;
    uint32_t nAwayOdds = 0;
    uint32_t nDrawOdds = 0;
    int32_t  nSpreadPoints = 0;  // Should be int16_t
    uint32_t nSpreadHomeOdds = 0;
    uint32_t nSpreadAwayOdds = 0;
    uint32_t nTotalPoints = 0;
    uint32_t nTotalOverOdds = 0;
    uint32_t nTotalUnderOdds = 0;
    uint32_t nMoneyLineHomePotentialLiability = 0;
    uint32_t nMoneyLineAwayPotentialLiability = 0;
    uint32_t nMoneyLineDrawPotentialLiability = 0;
    uint32_t nSpreadHomePotentialLiability = 0;
    uint32_t nSpreadAwayPotentialLiability = 0;
    uint32_t nSpreadPushPotentialLiability = 0;
    uint32_t nTotalOverPotentialLiability = 0;
    uint32_t nTotalUnderPotentialLiability = 0;
    uint32_t nTotalPushPotentialLiability = 0;
    uint32_t nMoneyLineHomeBets = 0;
    uint32_t nMoneyLineAwayBets = 0;
    uint32_t nMoneyLineDrawBets = 0;
    uint32_t nSpreadHomeBets = 0;
    uint32_t nSpreadAwayBets = 0;
    uint32_t nSpreadPushBets = 0;
    uint32_t nTotalOverBets = 0;
    uint32_t nTotalUnderBets = 0;
    uint32_t nTotalPushBets = 0;

    // Used in version 1 events
    int nEventCreationHeight = 0;
    bool fLegacyInitialHomeFavorite = true;

    // Default Constructor.
    CPeerlessEvent() {}

    static bool ToOpCode(CPeerlessEvent pe, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessEvent &pe);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nEventId);
        READWRITE(nStartTime);
        READWRITE(nSport);
        READWRITE(nTournament);
        READWRITE(nStage);
        READWRITE(nHomeTeam);
        READWRITE(nAwayTeam);
        READWRITE(nHomeOdds);
        READWRITE(nAwayOdds);
        READWRITE(nDrawOdds);
        READWRITE(nSpreadPoints);
        READWRITE(nSpreadHomeOdds);
        READWRITE(nSpreadAwayOdds);
        READWRITE(nTotalPoints);
        READWRITE(nTotalOverOdds);
        READWRITE(nTotalUnderOdds);
        READWRITE(nMoneyLineHomePotentialLiability);
        READWRITE(nMoneyLineAwayPotentialLiability);
        READWRITE(nMoneyLineDrawPotentialLiability);
        READWRITE(nSpreadHomePotentialLiability);
        READWRITE(nSpreadAwayPotentialLiability);
        READWRITE(nSpreadPushPotentialLiability);
        READWRITE(nTotalOverPotentialLiability);
        READWRITE(nTotalUnderPotentialLiability);
        READWRITE(nTotalPushPotentialLiability);
        READWRITE(nMoneyLineHomeBets);
        READWRITE(nMoneyLineAwayBets);
        READWRITE(nMoneyLineDrawBets);
        READWRITE(nSpreadHomeBets);
        READWRITE(nSpreadAwayBets);
        READWRITE(nSpreadPushBets);
        READWRITE(nTotalOverBets);
        READWRITE(nTotalUnderBets);
        READWRITE(nTotalPushBets);

        READWRITE(nEventCreationHeight);
        if (nEventCreationHeight < Params().WagerrProtocolV3StartHeight()) {
            READWRITE(fLegacyInitialHomeFavorite);
        }
    }
};

class CPeerlessBet
{
public:
    uint32_t nEventId;
    OutcomeType nOutcome;

    // Default constructor.
    CPeerlessBet() {}

    // Parametrized constructor.
    CPeerlessBet(int eventId, OutcomeType outcome)
    {
        nEventId = eventId;
        nOutcome = outcome;
    }
    bool operator<(const CPeerlessBet& rhs) const
    {
        return nEventId == rhs.nEventId ? nOutcome < rhs.nOutcome : nEventId < rhs.nEventId;
    }

    bool operator==(const CPeerlessBet& rhs) const
    {
        return nEventId == rhs.nEventId && nOutcome == rhs.nOutcome;
    }

    static bool ToOpCode(CPeerlessBet pb, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessBet &pb);
    static bool ParlayToOpCode(const std::vector<CPeerlessBet>& legs, std::string &opCode);
    static bool ParlayFromOpCode(const std::string& opCode, std::vector<CPeerlessBet>& legs);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint8_t outcome;
        READWRITE(nEventId);
        if (ser_action.ForRead()) {
            READWRITE(outcome);
            nOutcome = (OutcomeType) outcome;
        }
        else {
            outcome = (uint8_t) nOutcome;
            READWRITE(outcome);
        }
    }
};

class CPeerlessResult
{
public:
    uint32_t nEventId;
    uint32_t nResultType;
    uint32_t nHomeScore;
    uint32_t nAwayScore;


    // Default Constructor.
    CPeerlessResult() {}

    // Parametrized Constructor.
    CPeerlessResult(int eventId, int pResultType, int pHomeScore, int pAwayScore)
    {
        nEventId    = eventId;
        nResultType = pResultType;
        nHomeScore  = pHomeScore;
        nAwayScore  = pAwayScore;
    }

    static bool ToOpCode(CPeerlessResult pr, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessResult &pr);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nEventId);
        READWRITE(nResultType);
        READWRITE(nHomeScore);
        READWRITE(nAwayScore);
    }
};

class CPeerlessUpdateOdds
{
public:
    uint32_t nEventId;
    uint32_t nHomeOdds;
    uint32_t nAwayOdds;
    uint32_t nDrawOdds;

    // Default Constructor.
    CPeerlessUpdateOdds() {}

    static bool ToOpCode(CPeerlessUpdateOdds puo, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessUpdateOdds &puo);
};

class CChainGamesEvent
{
public:
    uint32_t nEventId;
    uint32_t nEntryFee;

    // Default Constructor.
    CChainGamesEvent() {}

    static bool ToOpCode(CChainGamesEvent cge, std::string &opCode);
    static bool FromOpCode(std::string opCode, CChainGamesEvent &cge);
};

class CChainGamesBet
{
public:
    uint32_t nEventId;

    // Default Constructor.
    CChainGamesBet() {}

    // Parametrized Constructor.
    CChainGamesBet(int eventId) {
        nEventId = eventId;
    }

    static bool ToOpCode(CChainGamesBet cgb, std::string &opCode);
    static bool FromOpCode(std::string opCode, CChainGamesBet &cgb);
};

class CChainGamesResult
{
public:
    uint16_t nEventId;

    // Default Constructor.
    CChainGamesResult() {}

    CChainGamesResult(uint16_t nEventId) : nEventId(nEventId) {};

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(nEventId);
    }

    bool FromScript(CScript script);

    static bool ToOpCode(CChainGamesResult cgr, std::string &opCode);
    static bool FromOpCode(std::string opCode, CChainGamesResult &cgr);
};

class CPeerlessSpreadsEvent
{
public:
    uint32_t nEventId;
    uint8_t nVersion;
    int32_t nPoints;    // Should be int16_t
    uint32_t nHomeOdds;
    uint32_t nAwayOdds;

    // Default Constructor.
    CPeerlessSpreadsEvent() {}

    static bool ToOpCode(CPeerlessSpreadsEvent pse, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessSpreadsEvent &pse);
};

class CPeerlessTotalsEvent
{
public:
    uint32_t nEventId;
    uint32_t nPoints;
    uint32_t nOverOdds;
    uint32_t nUnderOdds;

    // Default Constructor.
    CPeerlessTotalsEvent() {}

    static bool ToOpCode(CPeerlessTotalsEvent pte, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessTotalsEvent &pte);
};

class CPeerlessEventPatch
{
public:
    uint32_t nEventId;
    uint64_t nStartTime;

    CPeerlessEventPatch() {}

    static bool ToOpCode(CPeerlessEventPatch pe, std::string &opCode);
    static bool FromOpCode(std::string opCode, CPeerlessEventPatch &pe);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nEventId);
        READWRITE(nStartTime);
    }
};

class CMapping
{
public:

    uint32_t nMType;
    uint32_t nId;
    std::string sName;

    CMapping() {}

    MappingTypes GetType() const;

    static std::string ToTypeName(MappingTypes type);
    static MappingTypes FromTypeName(const std::string& name);

    static bool ToOpCode(const CMapping& mapping, std::string &opCode);
    static bool FromOpCode(std::string opCode, CMapping &mapping);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nMType);
        READWRITE(nId);
        READWRITE(sName);
    }
};

// OPCODE serialization class
class CQuickGamesTxBet
{
public:
    QuickGamesType gameType;
    std::vector<unsigned char> vBetInfo;

    static bool ToOpCode(CQuickGamesTxBet& bet, std::string &opCode);
    static bool FromOpCode(std::string opCode, CQuickGamesTxBet &bet);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint8_t type;
        if (ser_action.ForRead()) {
            READWRITE(type);
            gameType = (QuickGamesType) type;

        }
        else {
            type = (uint8_t) gameType;
            READWRITE(type);
        }
        READWRITE(vBetInfo);
    }
};

// DataBase Code

// MappingKey
typedef struct MappingKey {
    uint32_t nMType;
    uint32_t nId;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint32_t be_val;
        if (ser_action.ForRead()) {
            READWRITE(be_val);
            nMType = ntohl(be_val);
            READWRITE(be_val);
            nId = ntohl(be_val);
        }
        else {
            be_val = htonl(nMType);
            READWRITE(be_val);
            be_val = htonl(nId);
            READWRITE(be_val);
        }
    }
} MappingKey;

// EventKey
typedef struct EventKey {
    uint32_t eventId;

    explicit EventKey(uint32_t id) : eventId(id) { }
    explicit EventKey(const EventKey& key) : eventId(key.eventId) { }
    explicit EventKey(EventKey&& key) : eventId(key.eventId) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint32_t be_val;
        if (ser_action.ForRead()) {
            READWRITE(be_val);
            eventId = ntohl(be_val);
        }
        else {
            be_val = htonl(eventId);
            READWRITE(be_val);
        }
    }
} EventKey;

// ResultKey
using ResultKey = EventKey;

// UniversalBetKey
typedef struct UniversalBetKey {
    uint32_t blockHeight;
    COutPoint outPoint;

    explicit UniversalBetKey() : blockHeight(0), outPoint(COutPoint()) { }
    explicit UniversalBetKey(uint32_t height, COutPoint out) : blockHeight(height), outPoint(out) { }
    explicit UniversalBetKey(const UniversalBetKey& betKey) : blockHeight{betKey.blockHeight}, outPoint{betKey.outPoint} { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint32_t be_val;
        if (ser_action.ForRead()) {
            READWRITE(be_val);
            blockHeight = ntohl(be_val);
        }
        else {
            be_val = htonl(blockHeight);
            READWRITE(be_val);
        }
        READWRITE(outPoint);
    }
} UniversalBetKey;

// class for serializing bets on DB
class CUniversalBet
{
public:
    CAmount betAmount;
    CBitcoinAddress playerAddress;
    // one elem means single bet, else it is parlay bet, max size = 5
    std::vector<CPeerlessBet> legs;
    // vector for member event condition
    std::vector<CPeerlessEvent> lockedEvents;
    COutPoint betOutPoint;
    int64_t betTime;
    BetResultType resultType = BetResultType::betResultUnknown;
    CAmount payout = 0;

    explicit CUniversalBet() { }
    explicit CUniversalBet(const CAmount amount, const CBitcoinAddress address, const std::vector<CPeerlessBet> vLegs, const std::vector<CPeerlessEvent> vEvents, const COutPoint outPoint, const int64_t time) :
        betAmount(amount), playerAddress(address), legs(vLegs), lockedEvents(vEvents), betOutPoint(outPoint), betTime(time) { }
    explicit CUniversalBet(const CUniversalBet& bet)
    {
        betAmount = bet.betAmount;
        playerAddress = bet.playerAddress;
        legs = bet.legs;
        lockedEvents = bet.lockedEvents;
        betOutPoint = bet.betOutPoint;
        betTime = bet.betTime;
        completed = bet.completed;
        resultType = bet.resultType;
        payout = bet.payout;
    }

    bool IsCompleted() { return completed; }
    void SetCompleted() { completed = true; }
    // for undo
    void SetUncompleted() { completed = false; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        std::string addrStr;
        READWRITE(betAmount);
        if (ser_action.ForRead()) {
            READWRITE(addrStr);
            playerAddress.SetString(addrStr);
        }
        else {
            addrStr = playerAddress.ToString();
            READWRITE(addrStr);
        }
        READWRITE(legs);
        READWRITE(lockedEvents);
        READWRITE(betOutPoint);
        READWRITE(betTime);
        READWRITE(completed);
        uint8_t resType;
        if (ser_action.ForRead()) {
            READWRITE(resType);
            resultType = (BetResultType) resType;
        }
        else {
            resType = (uint8_t) resultType;
            READWRITE(resType);
        }
        READWRITE(payout);
    }

private:
    bool completed = false;
};

// Betting Undo

using BettingUndoKey = uint256;

using BettingUndoVariant = boost::variant<CMapping, CPeerlessEvent, CPeerlessResult>;

typedef enum BettingUndoTypes {
    UndoMapping,
    UndoPeerlessEvent,
    UndoPeerlessResult
} BettingUndoTypes;

class CBettingUndo
{
public:
    uint32_t height = 0;

    CBettingUndo() { }

    CBettingUndo(const BettingUndoVariant& undoVar, const uint32_t height) : height{height}, undoVariant{undoVar} { }

    bool Inited() {
        return !undoVariant.empty();
    }

    BettingUndoVariant Get() {
        return undoVariant;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(height);
        int undoType;
        if (ser_action.ForRead()) {
            READWRITE(undoType);
            switch ((BettingUndoTypes)undoType)
            {
                case UndoMapping:
                {
                    CMapping mapping{};
                    READWRITE(mapping);
                    undoVariant = mapping;
                    break;
                }
                case UndoPeerlessEvent:
                {
                    CPeerlessEvent event{};
                    READWRITE(event);
                    undoVariant = event;
                    break;
                }
                case UndoPeerlessResult:
                {
                    CPeerlessResult result{};
                    READWRITE(result);
                    undoVariant = result;
                    break;
                }
                default:
                    std::runtime_error("Undefined undo type");
            }
        }
        else {
            undoType = undoVariant.which();
            READWRITE(undoType);
            switch ((BettingUndoTypes)undoType)
            {
                case UndoMapping:
                {
                    CMapping mapping = boost::get<CMapping>(undoVariant);
                    READWRITE(mapping);
                    break;
                }
                case UndoPeerlessEvent:
                {
                    CPeerlessEvent event = boost::get<CPeerlessEvent>(undoVariant);
                    READWRITE(event);
                    break;
                }
                case UndoPeerlessResult:
                {
                    CPeerlessResult result = boost::get<CPeerlessResult>(undoVariant);
                    READWRITE(result);
                    break;
                }
                default:
                    std::runtime_error("Undefined undo type");
            }
        }
    }

private:
    BettingUndoVariant undoVariant;
};

// Payout Info

using PayoutInfoKey = UniversalBetKey;

class CPayoutInfo
{
public:
    UniversalBetKey betKey;
    PayoutType payoutType;

    explicit CPayoutInfo() { }
    explicit CPayoutInfo(UniversalBetKey &betKey, PayoutType payoutType) : betKey{betKey}, payoutType{payoutType} { }
    explicit CPayoutInfo(const CPayoutInfo& payoutInfo) : betKey{payoutInfo.betKey}, payoutType{payoutInfo.payoutType} { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(betKey);
        uint8_t type;
        if (ser_action.ForRead()) {
            READWRITE(type);
            payoutType = (PayoutType) type;

        }
        else {
            type = (uint8_t) payoutType;
            READWRITE(type);
        }
    }

    inline int CompareTo(const CPayoutInfo& rhs) const
    {
        if (betKey.blockHeight < rhs.betKey.blockHeight)
            return -1;
        if (betKey.blockHeight > rhs.betKey.blockHeight)
            return 1;
        if (betKey.outPoint < rhs.betKey.outPoint)
            return -1;
        if (betKey.outPoint != rhs.betKey.outPoint) // !(betKey.outPoint < rhs.betKey.outPoint) is demonstrated above
            return 1;
        if ((uint8_t)payoutType < (uint8_t)rhs.payoutType)
            return -1;
        if ((uint8_t)payoutType > (uint8_t)rhs.payoutType)
            return 1;
        return 0;
    }

    inline bool operator==(const CPayoutInfo& rhs) const { return CompareTo(rhs) == 0; }
    inline bool operator!=(const CPayoutInfo& rhs) const { return CompareTo(rhs) != 0; }
    inline bool operator<=(const CPayoutInfo& rhs) const { return CompareTo(rhs) <= 0; }
    inline bool operator>=(const CPayoutInfo& rhs) const { return CompareTo(rhs) >= 0; }
    inline bool operator<(const CPayoutInfo& rhs) const { return CompareTo(rhs) < 0; }
    inline bool operator>(const CPayoutInfo& rhs) const { return CompareTo(rhs) > 0; }
};

// Quick Games

using QuickGamesBetKey = UniversalBetKey;

class CQuickGamesBet
{
public:
    QuickGamesType gameType;
    std::vector<unsigned char> vBetInfo;
    CAmount betAmount;
    CBitcoinAddress playerAddress;
    int64_t betTime;
    BetResultType resultType = BetResultType::betResultUnknown;
    CAmount payout = 0;

    explicit CQuickGamesBet() { }
    explicit CQuickGamesBet(const QuickGamesType gameType, const std::vector<unsigned char>& vBetInfo, const CAmount betAmount, const CBitcoinAddress& playerAddress, const int64_t betTime) :
        gameType(gameType), vBetInfo(vBetInfo), betAmount(betAmount), playerAddress(playerAddress), betTime(betTime) { }
    explicit CQuickGamesBet(const CQuickGamesBet& cgBet) :
        gameType(cgBet.gameType), vBetInfo(cgBet.vBetInfo), betAmount(cgBet.betAmount), playerAddress(cgBet.playerAddress), betTime(cgBet.betTime), resultType(cgBet.resultType), payout(cgBet.payout), completed(cgBet.completed) { }

    bool IsCompleted() { return completed; }
    void SetCompleted() { completed = true; }
    // for undo
    void SetUncompleted() { completed = false; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp (Stream& s, Operation ser_action, int nType, int nVersion) {
        uint8_t type;
        std::string addrStr;
        if (ser_action.ForRead()) {
            READWRITE(type);
            gameType = (QuickGamesType) type;

        }
        else {
            type = (uint8_t) gameType;
            READWRITE(type);
        }
        READWRITE(vBetInfo);
        READWRITE(betAmount);
        if (ser_action.ForRead()) {
            READWRITE(addrStr);
            playerAddress.SetString(addrStr);
        }
        else {
            addrStr = playerAddress.ToString();
            READWRITE(addrStr);
        }
        READWRITE(betTime);
        uint8_t resType;
        if (ser_action.ForRead()) {
            READWRITE(resType);
            resultType = (BetResultType) resType;
        }
        else {
            resType = (uint8_t) resultType;
            READWRITE(resType);
        }
        READWRITE(payout);
        READWRITE(completed);
    }
private:
    bool completed = false;
};

// Betting Data Base

class CBettingDB
{
public:
    // Default Constructor.
    explicit CBettingDB(CStorageKV& db) : db{db} { }
    // Cache copy constructor (we should set global flushable storage ref as flushable storage of cached copy)
    explicit CBettingDB(CBettingDB& bdb) : CBettingDB(bdb.GetDb()) { }

    ~CBettingDB() {}

    bool Flush() { return db.Flush(); }

    std::unique_ptr<CStorageKVIterator> NewIterator() {
        return db.NewIterator();
    }

    template<typename KeyType>
    bool Exists(const KeyType& key) {
        return GetDb().Exists(DbTypeToBytes(key));
    }

    template<typename KeyType, typename ValueType>
    bool Write(const KeyType& key, const ValueType& value) {
        auto vKey = DbTypeToBytes(key);
        auto vValue = DbTypeToBytes(value);
        if (GetDb().Exists(vKey))
            return false;
        return GetDb().Write(vKey, vValue);
    }

    template<typename KeyType, typename ValueType>
    bool Update(const KeyType& key, const ValueType& value) {
        auto vKey = DbTypeToBytes(key);
        auto vValue = DbTypeToBytes(value);
        if (!GetDb().Exists(vKey))
            return false;
        return GetDb().Write(vKey, vValue);
    }

    template<typename KeyType>
    bool Erase(const KeyType& key) {
        auto vKey = DbTypeToBytes(key);
        if (!GetDb().Exists(vKey))
            return false;
        return GetDb().Erase(vKey);
    }

    template<typename KeyType, typename ValueType>
    bool Read(const KeyType& key, ValueType& value) {
        auto vKey = DbTypeToBytes(key);
        std::vector<unsigned char> vValue;
        if (GetDb().Read(vKey, vValue)) {
            BytesToDbType(vValue, value);
            return true;
        }
        return false;
    }

    unsigned int GetCacheSize() {
        return GetDb().GetCacheSize();
    }

    unsigned int GetCacheSizeBytesToWrite() {
        return GetDb().GetCacheSizeBytesToWrite();
    }

    static std::size_t dbWrapperCacheSize() { return 10 << 20; }

    static std::string MakeDbPath(const char* name) {
        using namespace boost::filesystem;

        std::string result{};
        path dir{GetDataDir()};

        dir /= "betting";
        dir /= name;

        if (boost::filesystem::is_directory(dir) || boost::filesystem::create_directories(dir) ) {
            result = boost::to_string(dir);
            result.erase(0, 1);
            result.erase(result.size() - 1);
        }

        return result;
    }

    template<typename T>
    static std::vector<unsigned char> DbTypeToBytes(const T& value) {
        CDataStream stream(SER_DISK, CLIENT_VERSION);
        stream << value;
        return std::vector<unsigned char>(stream.begin(), stream.end());
    }

    template<typename T>
    static void BytesToDbType(const std::vector<unsigned char>& bytes, T& value) {
        CDataStream stream(bytes, SER_DISK, CLIENT_VERSION);
        stream >> value;
        assert(stream.size() == 0);
    }
protected:
    CFlushableStorageKV& GetDb() { return db; }
    CFlushableStorageKV db;
};

/** Container for several db objects */
class CBettingsView
{
    // fields will be init in init.cpp
public:
    std::unique_ptr<CBettingDB> mappings; // "mappings"
    std::unique_ptr<CStorageKV> mappingsStorage;
    std::unique_ptr<CBettingDB> results; // "results"
    std::unique_ptr<CStorageKV> resultsStorage;
    std::unique_ptr<CBettingDB> events; // "events"
    std::unique_ptr<CStorageKV> eventsStorage;
    std::unique_ptr<CBettingDB> bets; // "bets"
    std::unique_ptr<CStorageKV> betsStorage;
    std::unique_ptr<CBettingDB> undos; // "undos"
    std::unique_ptr<CStorageKV> undosStorage;
    std::unique_ptr<CBettingDB> payoutsInfo; // "payoutsinfo"
    std::unique_ptr<CStorageKV> payoutsInfoStorage;
    std::unique_ptr<CBettingDB> quickGamesBets; // "quickgamesbets"
    std::unique_ptr<CStorageKV> quickGamesBetsStorage;

    // default constructor
    CBettingsView() { }

    // copy constructor for creating DB cache
    CBettingsView(CBettingsView* phr) {
        mappings = MakeUnique<CBettingDB>(*phr->mappings.get());
        results = MakeUnique<CBettingDB>(*phr->results.get());
        events = MakeUnique<CBettingDB>(*phr->events.get());
        bets = MakeUnique<CBettingDB>(*phr->bets.get());
        undos = MakeUnique<CBettingDB>(*phr->undos.get());
        payoutsInfo = MakeUnique<CBettingDB>(*phr->payoutsInfo.get());
        quickGamesBets = MakeUnique<CBettingDB>(*phr->quickGamesBets.get());
    }

    bool Flush() {
        return mappings->Flush() &&
                results->Flush() &&
                events->Flush() &&
                bets->Flush() &&
                undos->Flush() &&
                payoutsInfo->Flush() &&
                quickGamesBets->Flush();
    }

    unsigned int GetCacheSize() {
        return mappings->GetCacheSize() +
                results->GetCacheSize() +
                events->GetCacheSize() +
                bets->GetCacheSize() +
                undos->GetCacheSize() +
                payoutsInfo->GetCacheSize() +
                quickGamesBets->GetCacheSize();
    }

    unsigned int GetCacheSizeBytesToWrite() {
        return mappings->GetCacheSizeBytesToWrite() +
                results->GetCacheSizeBytesToWrite() +
                events->GetCacheSizeBytesToWrite() +
                bets->GetCacheSizeBytesToWrite() +
                undos->GetCacheSizeBytesToWrite() +
                payoutsInfo->GetCacheSizeBytesToWrite() +
                quickGamesBets->GetCacheSizeBytesToWrite();
    }

    void SetLastHeight(uint32_t height) {
        if (!undos->Exists(std::string("LastHeight"))) {
            undos->Write(std::string("LastHeight"), height);
        }
        else {
            undos->Update(std::string("LastHeight"), height);
        }
    }

    uint32_t GetLastHeight() {
        uint32_t height;
        if (!undos->Read(std::string("LastHeight"), height))
            return 0;
        return height;
    }

    bool SaveBettingUndo(const BettingUndoKey& key, std::vector<CBettingUndo> vUndos) {
        assert(!undos->Exists(key));
        return undos->Write(key, vUndos);
    }

    bool EraseBettingUndo(const BettingUndoKey& key) {
        return undos->Erase(key);
    }

    std::vector<CBettingUndo> GetBettingUndo(const BettingUndoKey& key) {
        std::vector<CBettingUndo> vUndos;
        if (undos->Read(key, vUndos))
            return vUndos;
        else
            return std::vector<CBettingUndo>{};
    }

    void PruneOlderUndos(const uint32_t height) {
        std::vector<CBettingUndo> vUndos;
        BettingUndoKey key;
        std::string str;
        auto it = undos->NewIterator();
        std::vector<BettingUndoKey> vKeysToDelete;
        std::vector<unsigned char> lastHeightKey = CBettingDB::DbTypeToBytes(std::string("LastHeight"));
        for (it->Seek(std::vector<unsigned char>{}); it->Valid(); it->Next()) {
            // check that key is serialized "LastHeight" key and skip if true
            if (it->Key() == lastHeightKey) {
                continue;
            }
            CBettingDB::BytesToDbType(it->Key(), key);
            CBettingDB::BytesToDbType(it->Value(), vUndos);
            if (vUndos[0].height < height) {
                vKeysToDelete.push_back(key);
            }
        }
        for (auto && key : vKeysToDelete) {
            undos->Erase(key);
        }
    }
};

extern CBettingsView *bettingsView;

/** Ensures a TX has come from an OMNO wallet. **/
bool IsValidOracleTx(const CTxIn &txin);

//* Calculates the amount of coins paid out to bettors and the amount of coins to burn, based on bet amount and odds **/
bool CalculatePayoutBurnAmounts(const CAmount betAmount, const uint32_t odds, CAmount& nPayout, CAmount& nBurn);

/** Validating the payout block using the payout vector. **/
bool IsBlockPayoutsValid(CBettingsView &bettingsViewCache, std::multimap<CPayoutInfo, CBetOut> mExpectedPayouts, const CBlock& block, int nBlockHeight, const CAmount& nExpectedMint, const CAmount& nMasternodeReward);

/** Find peerless events. **/
std::vector<CPeerlessResult> getEventResults(int height);

/** Find chain games lotto result. **/
std::pair<std::vector<CChainGamesResult>,std::vector<std::string>> getCGLottoEventResults(int height);

/** Undo bets as marked completed when generating payouts **/
bool UndoBetPayouts(CBettingsView &bettingsViewCache, int height);

/** Check Betting Tx when try accept tx to memory pool **/
bool CheckBettingTx(CBettingsView& bettingsViewCache, const CTransaction& tx, const int height);

/** Parse the transaction for betting data **/
void ParseBettingTx(CBettingsView& bettingsViewCache, const CTransaction& tx, const int height, const int64_t blockTime, const bool ParseBettingTx);

/** Get the chain height **/
int GetActiveChainHeight(const bool lockHeld = false);

bool RecoveryBettingDB(boost::signals2::signal<void(const std::string&)> & progress);

bool UndoBettingTx(CBettingsView& bettingsViewCache, const CTransaction& tx, const uint32_t height, const int64_t blockTime);

/* Revert payouts info from DB */
bool UndoPayoutsInfo(CBettingsView &bettingsViewCache, int height);

#endif // WAGERR_BET_H
