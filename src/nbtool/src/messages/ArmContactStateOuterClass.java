// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ArmContactState.proto

package messages;

public final class ArmContactStateOuterClass {
  private ArmContactStateOuterClass() {}
  public static void registerAllExtensions(
      com.google.protobuf.ExtensionRegistry registry) {
  }
  public interface ArmContactStateOrBuilder extends
      // @@protoc_insertion_point(interface_extends:messages.ArmContactState)
      com.google.protobuf.MessageOrBuilder {

    /**
     * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
     */
    boolean hasRightPushDirection();
    /**
     * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
     */
    messages.ArmContactStateOuterClass.ArmContactState.PushDirection getRightPushDirection();

    /**
     * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
     */
    boolean hasLeftPushDirection();
    /**
     * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
     */
    messages.ArmContactStateOuterClass.ArmContactState.PushDirection getLeftPushDirection();
  }
  /**
   * Protobuf type {@code messages.ArmContactState}
   */
  public static final class ArmContactState extends
      com.google.protobuf.GeneratedMessage implements
      // @@protoc_insertion_point(message_implements:messages.ArmContactState)
      ArmContactStateOrBuilder {
    // Use ArmContactState.newBuilder() to construct.
    private ArmContactState(com.google.protobuf.GeneratedMessage.Builder<?> builder) {
      super(builder);
      this.unknownFields = builder.getUnknownFields();
    }
    private ArmContactState(boolean noInit) { this.unknownFields = com.google.protobuf.UnknownFieldSet.getDefaultInstance(); }

    private static final ArmContactState defaultInstance;
    public static ArmContactState getDefaultInstance() {
      return defaultInstance;
    }

    public ArmContactState getDefaultInstanceForType() {
      return defaultInstance;
    }

    private final com.google.protobuf.UnknownFieldSet unknownFields;
    @java.lang.Override
    public final com.google.protobuf.UnknownFieldSet
        getUnknownFields() {
      return this.unknownFields;
    }
    private ArmContactState(
        com.google.protobuf.CodedInputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      initFields();
      int mutable_bitField0_ = 0;
      com.google.protobuf.UnknownFieldSet.Builder unknownFields =
          com.google.protobuf.UnknownFieldSet.newBuilder();
      try {
        boolean done = false;
        while (!done) {
          int tag = input.readTag();
          switch (tag) {
            case 0:
              done = true;
              break;
            default: {
              if (!parseUnknownField(input, unknownFields,
                                     extensionRegistry, tag)) {
                done = true;
              }
              break;
            }
            case 8: {
              int rawValue = input.readEnum();
              messages.ArmContactStateOuterClass.ArmContactState.PushDirection value = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.valueOf(rawValue);
              if (value == null) {
                unknownFields.mergeVarintField(1, rawValue);
              } else {
                bitField0_ |= 0x00000001;
                rightPushDirection_ = value;
              }
              break;
            }
            case 16: {
              int rawValue = input.readEnum();
              messages.ArmContactStateOuterClass.ArmContactState.PushDirection value = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.valueOf(rawValue);
              if (value == null) {
                unknownFields.mergeVarintField(2, rawValue);
              } else {
                bitField0_ |= 0x00000002;
                leftPushDirection_ = value;
              }
              break;
            }
          }
        }
      } catch (com.google.protobuf.InvalidProtocolBufferException e) {
        throw e.setUnfinishedMessage(this);
      } catch (java.io.IOException e) {
        throw new com.google.protobuf.InvalidProtocolBufferException(
            e.getMessage()).setUnfinishedMessage(this);
      } finally {
        this.unknownFields = unknownFields.build();
        makeExtensionsImmutable();
      }
    }
    public static final com.google.protobuf.Descriptors.Descriptor
        getDescriptor() {
      return messages.ArmContactStateOuterClass.internal_static_messages_ArmContactState_descriptor;
    }

    protected com.google.protobuf.GeneratedMessage.FieldAccessorTable
        internalGetFieldAccessorTable() {
      return messages.ArmContactStateOuterClass.internal_static_messages_ArmContactState_fieldAccessorTable
          .ensureFieldAccessorsInitialized(
              messages.ArmContactStateOuterClass.ArmContactState.class, messages.ArmContactStateOuterClass.ArmContactState.Builder.class);
    }

    public static com.google.protobuf.Parser<ArmContactState> PARSER =
        new com.google.protobuf.AbstractParser<ArmContactState>() {
      public ArmContactState parsePartialFrom(
          com.google.protobuf.CodedInputStream input,
          com.google.protobuf.ExtensionRegistryLite extensionRegistry)
          throws com.google.protobuf.InvalidProtocolBufferException {
        return new ArmContactState(input, extensionRegistry);
      }
    };

    @java.lang.Override
    public com.google.protobuf.Parser<ArmContactState> getParserForType() {
      return PARSER;
    }

    /**
     * Protobuf enum {@code messages.ArmContactState.PushDirection}
     */
    public enum PushDirection
        implements com.google.protobuf.ProtocolMessageEnum {
      /**
       * <code>NONE = 0;</code>
       */
      NONE(0, 0),
      /**
       * <code>NORTH = 1;</code>
       */
      NORTH(1, 1),
      /**
       * <code>NORTHEAST = 2;</code>
       */
      NORTHEAST(2, 2),
      /**
       * <code>EAST = 3;</code>
       */
      EAST(3, 3),
      /**
       * <code>SOUTHEAST = 4;</code>
       */
      SOUTHEAST(4, 4),
      /**
       * <code>SOUTH = 5;</code>
       */
      SOUTH(5, 5),
      /**
       * <code>SOUTHWEST = 6;</code>
       */
      SOUTHWEST(6, 6),
      /**
       * <code>WEST = 7;</code>
       */
      WEST(7, 7),
      /**
       * <code>NORTHWEST = 8;</code>
       */
      NORTHWEST(8, 8),
      ;

      /**
       * <code>NONE = 0;</code>
       */
      public static final int NONE_VALUE = 0;
      /**
       * <code>NORTH = 1;</code>
       */
      public static final int NORTH_VALUE = 1;
      /**
       * <code>NORTHEAST = 2;</code>
       */
      public static final int NORTHEAST_VALUE = 2;
      /**
       * <code>EAST = 3;</code>
       */
      public static final int EAST_VALUE = 3;
      /**
       * <code>SOUTHEAST = 4;</code>
       */
      public static final int SOUTHEAST_VALUE = 4;
      /**
       * <code>SOUTH = 5;</code>
       */
      public static final int SOUTH_VALUE = 5;
      /**
       * <code>SOUTHWEST = 6;</code>
       */
      public static final int SOUTHWEST_VALUE = 6;
      /**
       * <code>WEST = 7;</code>
       */
      public static final int WEST_VALUE = 7;
      /**
       * <code>NORTHWEST = 8;</code>
       */
      public static final int NORTHWEST_VALUE = 8;


      public final int getNumber() { return value; }

      public static PushDirection valueOf(int value) {
        switch (value) {
          case 0: return NONE;
          case 1: return NORTH;
          case 2: return NORTHEAST;
          case 3: return EAST;
          case 4: return SOUTHEAST;
          case 5: return SOUTH;
          case 6: return SOUTHWEST;
          case 7: return WEST;
          case 8: return NORTHWEST;
          default: return null;
        }
      }

      public static com.google.protobuf.Internal.EnumLiteMap<PushDirection>
          internalGetValueMap() {
        return internalValueMap;
      }
      private static com.google.protobuf.Internal.EnumLiteMap<PushDirection>
          internalValueMap =
            new com.google.protobuf.Internal.EnumLiteMap<PushDirection>() {
              public PushDirection findValueByNumber(int number) {
                return PushDirection.valueOf(number);
              }
            };

      public final com.google.protobuf.Descriptors.EnumValueDescriptor
          getValueDescriptor() {
        return getDescriptor().getValues().get(index);
      }
      public final com.google.protobuf.Descriptors.EnumDescriptor
          getDescriptorForType() {
        return getDescriptor();
      }
      public static final com.google.protobuf.Descriptors.EnumDescriptor
          getDescriptor() {
        return messages.ArmContactStateOuterClass.ArmContactState.getDescriptor().getEnumTypes().get(0);
      }

      private static final PushDirection[] VALUES = values();

      public static PushDirection valueOf(
          com.google.protobuf.Descriptors.EnumValueDescriptor desc) {
        if (desc.getType() != getDescriptor()) {
          throw new java.lang.IllegalArgumentException(
            "EnumValueDescriptor is not for this type.");
        }
        return VALUES[desc.getIndex()];
      }

      private final int index;
      private final int value;

      private PushDirection(int index, int value) {
        this.index = index;
        this.value = value;
      }

      // @@protoc_insertion_point(enum_scope:messages.ArmContactState.PushDirection)
    }

    private int bitField0_;
    public static final int RIGHT_PUSH_DIRECTION_FIELD_NUMBER = 1;
    private messages.ArmContactStateOuterClass.ArmContactState.PushDirection rightPushDirection_;
    /**
     * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
     */
    public boolean hasRightPushDirection() {
      return ((bitField0_ & 0x00000001) == 0x00000001);
    }
    /**
     * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
     */
    public messages.ArmContactStateOuterClass.ArmContactState.PushDirection getRightPushDirection() {
      return rightPushDirection_;
    }

    public static final int LEFT_PUSH_DIRECTION_FIELD_NUMBER = 2;
    private messages.ArmContactStateOuterClass.ArmContactState.PushDirection leftPushDirection_;
    /**
     * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
     */
    public boolean hasLeftPushDirection() {
      return ((bitField0_ & 0x00000002) == 0x00000002);
    }
    /**
     * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
     */
    public messages.ArmContactStateOuterClass.ArmContactState.PushDirection getLeftPushDirection() {
      return leftPushDirection_;
    }

    private void initFields() {
      rightPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
      leftPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
    }
    private byte memoizedIsInitialized = -1;
    public final boolean isInitialized() {
      byte isInitialized = memoizedIsInitialized;
      if (isInitialized == 1) return true;
      if (isInitialized == 0) return false;

      memoizedIsInitialized = 1;
      return true;
    }

    public void writeTo(com.google.protobuf.CodedOutputStream output)
                        throws java.io.IOException {
      getSerializedSize();
      if (((bitField0_ & 0x00000001) == 0x00000001)) {
        output.writeEnum(1, rightPushDirection_.getNumber());
      }
      if (((bitField0_ & 0x00000002) == 0x00000002)) {
        output.writeEnum(2, leftPushDirection_.getNumber());
      }
      getUnknownFields().writeTo(output);
    }

    private int memoizedSerializedSize = -1;
    public int getSerializedSize() {
      int size = memoizedSerializedSize;
      if (size != -1) return size;

      size = 0;
      if (((bitField0_ & 0x00000001) == 0x00000001)) {
        size += com.google.protobuf.CodedOutputStream
          .computeEnumSize(1, rightPushDirection_.getNumber());
      }
      if (((bitField0_ & 0x00000002) == 0x00000002)) {
        size += com.google.protobuf.CodedOutputStream
          .computeEnumSize(2, leftPushDirection_.getNumber());
      }
      size += getUnknownFields().getSerializedSize();
      memoizedSerializedSize = size;
      return size;
    }

    private static final long serialVersionUID = 0L;
    @java.lang.Override
    protected java.lang.Object writeReplace()
        throws java.io.ObjectStreamException {
      return super.writeReplace();
    }

    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        com.google.protobuf.ByteString data)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        com.google.protobuf.ByteString data,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data, extensionRegistry);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(byte[] data)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        byte[] data,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws com.google.protobuf.InvalidProtocolBufferException {
      return PARSER.parseFrom(data, extensionRegistry);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(java.io.InputStream input)
        throws java.io.IOException {
      return PARSER.parseFrom(input);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        java.io.InputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return PARSER.parseFrom(input, extensionRegistry);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseDelimitedFrom(java.io.InputStream input)
        throws java.io.IOException {
      return PARSER.parseDelimitedFrom(input);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseDelimitedFrom(
        java.io.InputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return PARSER.parseDelimitedFrom(input, extensionRegistry);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        com.google.protobuf.CodedInputStream input)
        throws java.io.IOException {
      return PARSER.parseFrom(input);
    }
    public static messages.ArmContactStateOuterClass.ArmContactState parseFrom(
        com.google.protobuf.CodedInputStream input,
        com.google.protobuf.ExtensionRegistryLite extensionRegistry)
        throws java.io.IOException {
      return PARSER.parseFrom(input, extensionRegistry);
    }

    public static Builder newBuilder() { return Builder.create(); }
    public Builder newBuilderForType() { return newBuilder(); }
    public static Builder newBuilder(messages.ArmContactStateOuterClass.ArmContactState prototype) {
      return newBuilder().mergeFrom(prototype);
    }
    public Builder toBuilder() { return newBuilder(this); }

    @java.lang.Override
    protected Builder newBuilderForType(
        com.google.protobuf.GeneratedMessage.BuilderParent parent) {
      Builder builder = new Builder(parent);
      return builder;
    }
    /**
     * Protobuf type {@code messages.ArmContactState}
     */
    public static final class Builder extends
        com.google.protobuf.GeneratedMessage.Builder<Builder> implements
        // @@protoc_insertion_point(builder_implements:messages.ArmContactState)
        messages.ArmContactStateOuterClass.ArmContactStateOrBuilder {
      public static final com.google.protobuf.Descriptors.Descriptor
          getDescriptor() {
        return messages.ArmContactStateOuterClass.internal_static_messages_ArmContactState_descriptor;
      }

      protected com.google.protobuf.GeneratedMessage.FieldAccessorTable
          internalGetFieldAccessorTable() {
        return messages.ArmContactStateOuterClass.internal_static_messages_ArmContactState_fieldAccessorTable
            .ensureFieldAccessorsInitialized(
                messages.ArmContactStateOuterClass.ArmContactState.class, messages.ArmContactStateOuterClass.ArmContactState.Builder.class);
      }

      // Construct using messages.ArmContactStateOuterClass.ArmContactState.newBuilder()
      private Builder() {
        maybeForceBuilderInitialization();
      }

      private Builder(
          com.google.protobuf.GeneratedMessage.BuilderParent parent) {
        super(parent);
        maybeForceBuilderInitialization();
      }
      private void maybeForceBuilderInitialization() {
        if (com.google.protobuf.GeneratedMessage.alwaysUseFieldBuilders) {
        }
      }
      private static Builder create() {
        return new Builder();
      }

      public Builder clear() {
        super.clear();
        rightPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
        bitField0_ = (bitField0_ & ~0x00000001);
        leftPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
        bitField0_ = (bitField0_ & ~0x00000002);
        return this;
      }

      public Builder clone() {
        return create().mergeFrom(buildPartial());
      }

      public com.google.protobuf.Descriptors.Descriptor
          getDescriptorForType() {
        return messages.ArmContactStateOuterClass.internal_static_messages_ArmContactState_descriptor;
      }

      public messages.ArmContactStateOuterClass.ArmContactState getDefaultInstanceForType() {
        return messages.ArmContactStateOuterClass.ArmContactState.getDefaultInstance();
      }

      public messages.ArmContactStateOuterClass.ArmContactState build() {
        messages.ArmContactStateOuterClass.ArmContactState result = buildPartial();
        if (!result.isInitialized()) {
          throw newUninitializedMessageException(result);
        }
        return result;
      }

      public messages.ArmContactStateOuterClass.ArmContactState buildPartial() {
        messages.ArmContactStateOuterClass.ArmContactState result = new messages.ArmContactStateOuterClass.ArmContactState(this);
        int from_bitField0_ = bitField0_;
        int to_bitField0_ = 0;
        if (((from_bitField0_ & 0x00000001) == 0x00000001)) {
          to_bitField0_ |= 0x00000001;
        }
        result.rightPushDirection_ = rightPushDirection_;
        if (((from_bitField0_ & 0x00000002) == 0x00000002)) {
          to_bitField0_ |= 0x00000002;
        }
        result.leftPushDirection_ = leftPushDirection_;
        result.bitField0_ = to_bitField0_;
        onBuilt();
        return result;
      }

      public Builder mergeFrom(com.google.protobuf.Message other) {
        if (other instanceof messages.ArmContactStateOuterClass.ArmContactState) {
          return mergeFrom((messages.ArmContactStateOuterClass.ArmContactState)other);
        } else {
          super.mergeFrom(other);
          return this;
        }
      }

      public Builder mergeFrom(messages.ArmContactStateOuterClass.ArmContactState other) {
        if (other == messages.ArmContactStateOuterClass.ArmContactState.getDefaultInstance()) return this;
        if (other.hasRightPushDirection()) {
          setRightPushDirection(other.getRightPushDirection());
        }
        if (other.hasLeftPushDirection()) {
          setLeftPushDirection(other.getLeftPushDirection());
        }
        this.mergeUnknownFields(other.getUnknownFields());
        return this;
      }

      public final boolean isInitialized() {
        return true;
      }

      public Builder mergeFrom(
          com.google.protobuf.CodedInputStream input,
          com.google.protobuf.ExtensionRegistryLite extensionRegistry)
          throws java.io.IOException {
        messages.ArmContactStateOuterClass.ArmContactState parsedMessage = null;
        try {
          parsedMessage = PARSER.parsePartialFrom(input, extensionRegistry);
        } catch (com.google.protobuf.InvalidProtocolBufferException e) {
          parsedMessage = (messages.ArmContactStateOuterClass.ArmContactState) e.getUnfinishedMessage();
          throw e;
        } finally {
          if (parsedMessage != null) {
            mergeFrom(parsedMessage);
          }
        }
        return this;
      }
      private int bitField0_;

      private messages.ArmContactStateOuterClass.ArmContactState.PushDirection rightPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
      /**
       * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
       */
      public boolean hasRightPushDirection() {
        return ((bitField0_ & 0x00000001) == 0x00000001);
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
       */
      public messages.ArmContactStateOuterClass.ArmContactState.PushDirection getRightPushDirection() {
        return rightPushDirection_;
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
       */
      public Builder setRightPushDirection(messages.ArmContactStateOuterClass.ArmContactState.PushDirection value) {
        if (value == null) {
          throw new NullPointerException();
        }
        bitField0_ |= 0x00000001;
        rightPushDirection_ = value;
        onChanged();
        return this;
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection right_push_direction = 1;</code>
       */
      public Builder clearRightPushDirection() {
        bitField0_ = (bitField0_ & ~0x00000001);
        rightPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
        onChanged();
        return this;
      }

      private messages.ArmContactStateOuterClass.ArmContactState.PushDirection leftPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
      /**
       * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
       */
      public boolean hasLeftPushDirection() {
        return ((bitField0_ & 0x00000002) == 0x00000002);
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
       */
      public messages.ArmContactStateOuterClass.ArmContactState.PushDirection getLeftPushDirection() {
        return leftPushDirection_;
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
       */
      public Builder setLeftPushDirection(messages.ArmContactStateOuterClass.ArmContactState.PushDirection value) {
        if (value == null) {
          throw new NullPointerException();
        }
        bitField0_ |= 0x00000002;
        leftPushDirection_ = value;
        onChanged();
        return this;
      }
      /**
       * <code>optional .messages.ArmContactState.PushDirection left_push_direction = 2;</code>
       */
      public Builder clearLeftPushDirection() {
        bitField0_ = (bitField0_ & ~0x00000002);
        leftPushDirection_ = messages.ArmContactStateOuterClass.ArmContactState.PushDirection.NONE;
        onChanged();
        return this;
      }

      // @@protoc_insertion_point(builder_scope:messages.ArmContactState)
    }

    static {
      defaultInstance = new ArmContactState(true);
      defaultInstance.initFields();
    }

    // @@protoc_insertion_point(class_scope:messages.ArmContactState)
  }

  private static final com.google.protobuf.Descriptors.Descriptor
    internal_static_messages_ArmContactState_descriptor;
  private static
    com.google.protobuf.GeneratedMessage.FieldAccessorTable
      internal_static_messages_ArmContactState_fieldAccessorTable;

  public static com.google.protobuf.Descriptors.FileDescriptor
      getDescriptor() {
    return descriptor;
  }
  private static com.google.protobuf.Descriptors.FileDescriptor
      descriptor;
  static {
    java.lang.String[] descriptorData = {
      "\n\025ArmContactState.proto\022\010messages\"\237\002\n\017Ar" +
      "mContactState\022E\n\024right_push_direction\030\001 " +
      "\001(\0162\'.messages.ArmContactState.PushDirec" +
      "tion\022D\n\023left_push_direction\030\002 \001(\0162\'.mess" +
      "ages.ArmContactState.PushDirection\"\177\n\rPu" +
      "shDirection\022\010\n\004NONE\020\000\022\t\n\005NORTH\020\001\022\r\n\tNORT" +
      "HEAST\020\002\022\010\n\004EAST\020\003\022\r\n\tSOUTHEAST\020\004\022\t\n\005SOUT" +
      "H\020\005\022\r\n\tSOUTHWEST\020\006\022\010\n\004WEST\020\007\022\r\n\tNORTHWES" +
      "T\020\010"
    };
    com.google.protobuf.Descriptors.FileDescriptor.InternalDescriptorAssigner assigner =
        new com.google.protobuf.Descriptors.FileDescriptor.    InternalDescriptorAssigner() {
          public com.google.protobuf.ExtensionRegistry assignDescriptors(
              com.google.protobuf.Descriptors.FileDescriptor root) {
            descriptor = root;
            return null;
          }
        };
    com.google.protobuf.Descriptors.FileDescriptor
      .internalBuildGeneratedFileFrom(descriptorData,
        new com.google.protobuf.Descriptors.FileDescriptor[] {
        }, assigner);
    internal_static_messages_ArmContactState_descriptor =
      getDescriptor().getMessageTypes().get(0);
    internal_static_messages_ArmContactState_fieldAccessorTable = new
      com.google.protobuf.GeneratedMessage.FieldAccessorTable(
        internal_static_messages_ArmContactState_descriptor,
        new java.lang.String[] { "RightPushDirection", "LeftPushDirection", });
  }

  // @@protoc_insertion_point(outer_class_scope)
}
